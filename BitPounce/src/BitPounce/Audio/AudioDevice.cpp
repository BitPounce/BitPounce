#include "bp_pch.h"
#include "AudioDevice.h"

#include "miniaudio.h"

#include <vector>
#include <atomic>
#include <mutex>
#include <optional>
#include <array>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>

namespace BitPounce
{
	static BitPouncePack::AudioFormat ToPackFormat(ma_format format)
	{
		switch (format)
		{
			case ma_format_u8:  return BitPouncePack::AudioFormat::u8;
			case ma_format_s16: return BitPouncePack::AudioFormat::s16;
			case ma_format_s24: return BitPouncePack::AudioFormat::s24;
			case ma_format_s32: return BitPouncePack::AudioFormat::s32;
			case ma_format_f32: return BitPouncePack::AudioFormat::f32;
			default:            return BitPouncePack::AudioFormat::unknown;
		}
	}

	// ---------- Constants ----------
	constexpr size_t MAX_COMMANDS = 64;
	constexpr float  MAX_VOLUME   = 1.0f;
	constexpr ma_uint32 PERIOD_SIZE_FRAMES = 2048 * 2;
	constexpr ma_uint32 PERIOD_COUNT       = 3 * 2;

	// ---------- Audio Source ----------
	struct AudioSource
	{
		std::vector<std::byte> memoryBuffer;
		ma_decoder decoder;
		ma_data_converter converter;

		std::vector<float> decodeBuffer;
		std::vector<float> convertBuffer;

		float  volume = 1.0f;
		bool   loop   = false;
		bool   playing = false;
		bool   atEnd   = false;

		AudioSource() = default;

		// No copy or move – miniaudio objects cannot be safely copied/moved
		AudioSource(const AudioSource&) = delete;
		AudioSource& operator=(const AudioSource&) = delete;
		AudioSource(AudioSource&&) = delete;
		AudioSource& operator=(AudioSource&&) = delete;

		~AudioSource()
		{
			ma_decoder_uninit(&decoder);
			ma_data_converter_uninit(&converter, nullptr);
		}

		bool allocateBuffers(ma_uint32 maxFrames, ma_uint32 deviceChannels, ma_uint32 sourceChannels)
		{
			try {
				decodeBuffer.resize(maxFrames * sourceChannels);
				convertBuffer.resize(maxFrames * deviceChannels);
			} catch (...) {
				return false;
			}
			return true;
		}
	};

	// ---------- Lock‑Free Command Queue ----------
	class CommandQueue
	{
	public:
		enum class Type : uint8_t
		{
			None,
			Play,
			Stop,
			SetVolume,
			SetLooping
		};

		struct Command
		{
			Type   type   = Type::None;
			uint64_t handle = 0;
			float    value = 0.0f;
			bool     flag  = false;
		};

		CommandQueue() : head(0), tail(0) {}

		bool push(const Command& cmd)
		{
			size_t currentTail = tail.load(std::memory_order_relaxed);
			size_t nextTail = (currentTail + 1) % MAX_COMMANDS;
			if (nextTail == head.load(std::memory_order_acquire))
				return false;

			buffer[currentTail] = cmd;
			tail.store(nextTail, std::memory_order_release);
			return true;
		}

		std::optional<Command> pop()
		{
			size_t currentHead = head.load(std::memory_order_relaxed);
			if (currentHead == tail.load(std::memory_order_acquire))
				return std::nullopt;

			Command cmd = buffer[currentHead];
			head.store((currentHead + 1) % MAX_COMMANDS, std::memory_order_release);
			return cmd;
		}

	private:
		std::array<Command, MAX_COMMANDS> buffer;
		std::atomic<size_t> head;
		std::atomic<size_t> tail;
	};

	struct AudioDeviceData
	{
		float worldVolume = 0.5f;
		ma_device device;
		bool initialized = false;
		ma_uint32 maxFramesPerCallback = PERIOD_SIZE_FRAMES;

		std::mutex sourceMutex;
		std::vector<AudioSource*> sources;   // raw pointers – manual management

		CommandQueue cmdQueue;
	};

	static AudioDeviceData* s_Data = nullptr;

	static inline float SoftClip(float x)
	{
		//if (x > 1.0f) return 1.0f;
		//if (x < -1.0f) return -1.0f;
		//return x * (1.5f - 0.5f * x * x);
		return x;
	}

	static void DataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount)
	{
		auto* data = (AudioDeviceData*)device->pUserData;
		float* out = (float*)output;
		ma_uint32 channels = device->playback.channels;

		std::memset(out, 0, frameCount * channels * sizeof(float));

		if (!data->sourceMutex.try_lock())
			return;

		// Process commands
		while (auto cmdOpt = data->cmdQueue.pop())
		{
			const auto& cmd = *cmdOpt;
			uint64_t rawHandle = cmd.handle;
			if (rawHandle == 0) continue;
			size_t idx = static_cast<size_t>(rawHandle - 1);
			if (idx >= data->sources.size())
				continue;

			auto* src = data->sources[idx];
			if (!src) continue;   // safety

			switch (cmd.type)
			{
			case CommandQueue::Type::Play:
				if (!src->playing)
				{
					ma_decoder_seek_to_pcm_frame(&src->decoder, 0);
					ma_data_converter_reset(&src->converter);   // crucial!
					src->playing = true;
					src->atEnd = false;
				}
				break;
			case CommandQueue::Type::Stop:
				src->playing = false;
				break;
			case CommandQueue::Type::SetVolume:
				src->volume = std::clamp(cmd.value, 0.0f, MAX_VOLUME);
				break;
			case CommandQueue::Type::SetLooping:
				src->loop = cmd.flag;
				break;
			default:
				break;
			}
		}

		for (size_t i = 0; i < data->sources.size(); ++i)
		{
			auto* src = data->sources[i];
			if (!src || !src->playing)
				continue;

			ma_uint64 framesRemaining = frameCount;
			float* outPtr = out;

			while (framesRemaining > 0)
			{
				if (src->atEnd)
				{
					if (src->loop)
					{
						ma_decoder_seek_to_pcm_frame(&src->decoder, 0);
						ma_data_converter_reset(&src->converter);
						src->atEnd = false;
					}
					else
					{
						src->playing = false;
						break;
					}
				}

				ma_uint64 framesToDecode = std::min<ma_uint64>(framesRemaining, data->maxFramesPerCallback);
				ma_uint64 framesRead = 0;
				ma_result result = ma_decoder_read_pcm_frames(
					&src->decoder,
					src->decodeBuffer.data(),
					framesToDecode,
					&framesRead
				);

				if (result != MA_SUCCESS || framesRead == 0)
				{
					src->atEnd = true;
					continue;
				}

				ma_uint64 framesToConvert = framesRead;
				ma_uint64 framesConverted = data->maxFramesPerCallback;
				ma_data_converter_process_pcm_frames(
					&src->converter,
					src->decodeBuffer.data(),
					&framesToConvert,
					src->convertBuffer.data(),
					&framesConverted
				);

				ma_uint32 samplesToMix = static_cast<ma_uint32>(framesConverted * channels);
				for (ma_uint32 s = 0; s < samplesToMix; ++s)
				{
					outPtr[s] += src->convertBuffer[s] * src->volume;
				}

				framesRemaining -= framesConverted;
				outPtr += framesConverted * channels;

				if (framesToConvert < framesToDecode)
					break;
			}
		}

		ma_uint32 totalSamples = frameCount * channels;
		for (ma_uint32 i = 0; i < totalSamples; ++i)
		{
			out[i] = SoftClip(out[i]);
			out[i] *= data->worldVolume;
		}

		

		data->sourceMutex.unlock();
	}
	
	AudioID AudioDevice::Load(const std::filesystem::path& filepath, bool loop)
	{
		if (!s_Data || !s_Data->initialized)
			return 0;

		auto* source = new AudioSource();   // raw allocation

		ma_result initErrorCode = ma_decoder_init_file(filepath.string().c_str(), nullptr, &source->decoder);
		if (initErrorCode != MA_SUCCESS)
		{
			delete source;
			return 0;
		}

		ma_data_converter_config converterConfig =
			ma_data_converter_config_init(
				source->decoder.outputFormat,
				ma_format_f32,
				source->decoder.outputChannels,
				s_Data->device.playback.channels,
				source->decoder.outputSampleRate,
				s_Data->device.sampleRate
			);
		if (ma_data_converter_init(&converterConfig, nullptr, &source->converter) != MA_SUCCESS)
		{
			ma_decoder_uninit(&source->decoder);
			delete source;
			return 0;
		}

		if (!source->allocateBuffers(
				s_Data->maxFramesPerCallback,
				s_Data->device.playback.channels,
				source->decoder.outputChannels))
		{
			ma_data_converter_uninit(&source->converter, nullptr);
			ma_decoder_uninit(&source->decoder);
			delete source;
			return 0;
		}

		source->loop = loop;
		source->playing = false;
		source->atEnd = false;

		size_t index;
		{
			std::lock_guard<std::mutex> lock(s_Data->sourceMutex);
			index = s_Data->sources.size();
			s_Data->sources.push_back(source);
		}

		return static_cast<AudioID>(index + 1);
	}

	void AudioDevice::Init()
	{
		if (s_Data != nullptr)
			return;

		auto* data = new AudioDeviceData();

		ma_device_config config = ma_device_config_init(ma_device_type_playback);
		config.playback.format   = ma_format_f32;
		config.playback.channels = 0;               // stereo
		config.sampleRate        = 48000;                // 48 kHz
		config.periodSizeInFrames = PERIOD_SIZE_FRAMES;
		config.periods            = PERIOD_COUNT;
		config.dataCallback      = DataCallback;
		config.pUserData         = data;

		if (ma_device_init(nullptr, &config, &data->device) != MA_SUCCESS)
		{
			delete data;
			return;
		}

		data->maxFramesPerCallback = PERIOD_SIZE_FRAMES;
		data->initialized = true;

		if (ma_device_start(&data->device) != MA_SUCCESS)
		{
			ma_device_uninit(&data->device);
			delete data;
			return;
		}

		s_Data = data;
	}

	void AudioDevice::UnInit()
	{
		if (!s_Data)
			return;

		ma_device_uninit(&s_Data->device);

		// Manually delete all sources
		{
			std::lock_guard<std::mutex> lock(s_Data->sourceMutex);
			for (auto* src : s_Data->sources)
				delete src;
			s_Data->sources.clear();
		}

		delete s_Data;
		s_Data = nullptr;
	}

	AudioID AudioDevice::Load(const BitPouncePack::PackAudio& pack, bool loop)
	{
		if (!s_Data || !s_Data->initialized || !pack.Data || pack.Size == 0)
			return 0;

		auto* source = new AudioSource();

		// Just for the hell of it, make a wave file.
		uint32_t bytesPerSample = 0;
		uint16_t audioFormat = 1; // PCM
		switch (pack.format) {
			case BitPouncePack::AudioFormat::u8:  bytesPerSample = 1; break;
			case BitPouncePack::AudioFormat::s16: bytesPerSample = 2; break;
			case BitPouncePack::AudioFormat::s24: bytesPerSample = 3; break;
			case BitPouncePack::AudioFormat::s32: bytesPerSample = 4; break;
			case BitPouncePack::AudioFormat::f32: bytesPerSample = 4; audioFormat = 3; break; // IEEE float
			default: delete source; return 0;
		}

		uint16_t bitsPerSample = static_cast<uint16_t>(bytesPerSample * 8);
		uint16_t blockAlign    = static_cast<uint16_t>(pack.Channels * bytesPerSample);
		uint32_t byteRate      = pack.SampleRate * blockAlign;
		uint32_t dataSize      = static_cast<uint32_t>(pack.Size);
		uint32_t chunkSize     = 36 + dataSize;

		struct WAVHeader {
			uint32_t chunkID       = 0x46464952; // "RIFF" do RIFF has rizz??
			uint32_t chunkSize;
			uint32_t format        = 0x45564157; // "WAVE"
			uint32_t subchunk1ID   = 0x20746D66; // "fmt "
			uint32_t subchunk1Size = 16;
			uint16_t audioFormat;
			uint16_t numChannels;
			uint32_t sampleRate;
			uint32_t byteRate;
			uint16_t blockAlign;
			uint16_t bitsPerSample;
			uint32_t subchunk2ID   = 0x61746164; // "data"
			uint32_t subchunk2Size;
		} header;

		header.chunkSize     = chunkSize;
		header.audioFormat   = audioFormat;
		header.numChannels   = static_cast<uint16_t>(pack.Channels);
		header.sampleRate    = pack.SampleRate;
		header.byteRate      = byteRate;
		header.blockAlign    = blockAlign;
		header.bitsPerSample = bitsPerSample;
		header.subchunk2Size = dataSize;

		// Allocate combined buffer (WAV header + raw PCM)
		source->memoryBuffer.resize(sizeof(header) + dataSize);
		std::byte* bufferPtr = source->memoryBuffer.data();
		std::memcpy(bufferPtr, &header, sizeof(header));
		std::memcpy(bufferPtr + sizeof(header), pack.Data.get(), dataSize);

		// Its time for a dwcoder!
		ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_unknown, 0, 0);
		if (ma_decoder_init_memory(bufferPtr, source->memoryBuffer.size(), &decoderConfig, &source->decoder) != MA_SUCCESS) {
			delete source;
			return 0;
		}

		// converter the data to hell
		ma_data_converter_config converterConfig =
			ma_data_converter_config_init(
				source->decoder.outputFormat,
				ma_format_f32,
				source->decoder.outputChannels,
				s_Data->device.playback.channels,
				source->decoder.outputSampleRate,
				s_Data->device.sampleRate
			);

		if (ma_data_converter_init(&converterConfig, nullptr, &source->converter) != MA_SUCCESS) {
			ma_decoder_uninit(&source->decoder);
			delete source;
			return 0;
		}

		if (!source->allocateBuffers(
				s_Data->maxFramesPerCallback,
				s_Data->device.playback.channels,
				source->decoder.outputChannels)) {
			ma_data_converter_uninit(&source->converter, nullptr);
			ma_decoder_uninit(&source->decoder);
			delete source;
			return 0;
		}

		source->loop    = loop;
		source->playing = false;
		source->atEnd   = false;

		size_t index;
		{
			std::lock_guard<std::mutex> lock(s_Data->sourceMutex);
			index = s_Data->sources.size();
			s_Data->sources.push_back(source);
		}

		return static_cast<AudioID>(index + 1);
	}

	void AudioDevice::Play(AudioID id)
	{
		if (!s_Data || !s_Data->initialized || id == 0)
			return;

		CommandQueue::Command cmd;
		cmd.type = CommandQueue::Type::Play;
		cmd.handle = id;
		s_Data->cmdQueue.push(cmd);
	}

	void AudioDevice::Stop(AudioID id)
	{
		if (!s_Data || !s_Data->initialized || id == 0)
			return;

		CommandQueue::Command cmd;
		cmd.type = CommandQueue::Type::Stop;
		cmd.handle = id;
		s_Data->cmdQueue.push(cmd);
	}

	void AudioDevice::SetVolume(AudioID id, float volume)
	{
		if (!s_Data || !s_Data->initialized || id == 0)
			return;

		CommandQueue::Command cmd;
		cmd.type = CommandQueue::Type::SetVolume;
		cmd.handle = id;
		cmd.value = volume;
		s_Data->cmdQueue.push(cmd);
	}

	void AudioDevice::SetLooping(AudioID id, bool loop)
	{
		if (!s_Data || !s_Data->initialized || id == 0)
			return;

		CommandQueue::Command cmd;
		cmd.type = CommandQueue::Type::SetLooping;
		cmd.handle = id;
		cmd.flag = loop;
		s_Data->cmdQueue.push(cmd);
	}
	bool AudioDevice::UnLoad(AudioID id)
	{

		if (!s_Data || !s_Data->initialized || id == 0)
			return false;

		size_t idx = static_cast<size_t>(id - 1);

		std::lock_guard<std::mutex> lock(s_Data->sourceMutex);

		if (idx >= s_Data->sources.size())
			return false;

		AudioSource* src = s_Data->sources[idx];
		if (!src)
			return false;   // already unloaded

		delete src;
		s_Data->sources[idx] = nullptr;   // leave a hole
		return true;
	}
	void AudioDevice::SetWorldVolume(float volume)
	{
		s_Data->worldVolume = volume;
	}
    float AudioDevice::GetWorldVolume()
    {
        return s_Data->worldVolume;
    }
    BitPouncePack::PackAudio AudioDevice::AudioToPackAudio(AudioID id)
    {
		if (!s_Data || !s_Data->initialized || id == 0)
			return {};

		size_t idx = static_cast<size_t>(id - 1);
		std::lock_guard<std::mutex> lock(s_Data->sourceMutex);

		if (idx >= s_Data->sources.size())
			return {};

		AudioSource* src = s_Data->sources[idx];
		if (!src)
			return {};

		ma_decoder* decoder = &src->decoder;

		// Get total PCM frame count
		ma_uint64 totalFrames;
		if (ma_decoder_get_length_in_pcm_frames(decoder, &totalFrames) != MA_SUCCESS)
			return {};

		// Query audio properties
		ma_uint32 channels       = decoder->outputChannels;
		ma_uint32 sampleRate     = decoder->outputSampleRate;
		ma_format srcFormat      = decoder->outputFormat;
		BitPouncePack::AudioFormat packFormat = ToPackFormat(srcFormat);
		if (packFormat == BitPouncePack::AudioFormat::unknown)
			return {};

		size_t bytesPerSample = ma_get_bytes_per_sample(srcFormat);
		size_t bytesPerFrame  = bytesPerSample * channels;
		size_t totalBytes     = static_cast<size_t>(totalFrames * bytesPerFrame);

		// Prepare PackAudio result
		BitPouncePack::PackAudio result;
		result.Data = std::make_unique<std::byte[]>(totalBytes);
		result.Size       = totalBytes;
		result.Channels   = channels;
		result.SampleRate = sampleRate;
		result.format     = packFormat;
		result.HashType   = 0;                     // 0 = SHA-256
		result.AssetHandle = static_cast<uint64_t>(id);

		// Save current decoder position, then seek to start
		ma_uint64 originalPos;
		if (ma_decoder_get_cursor_in_pcm_frames(decoder, &originalPos) != MA_SUCCESS)
			return {};
		if (ma_decoder_seek_to_pcm_frame(decoder, 0) != MA_SUCCESS)
			return {};

		// Read all PCM frames into the buffer
		uint8_t* writePtr = reinterpret_cast<uint8_t*>(result.Data.get());
		ma_uint64 framesRemaining = totalFrames;
		while (framesRemaining > 0)
		{
			ma_uint64 framesToRead = framesRemaining;
			ma_result res = ma_decoder_read_pcm_frames(decoder, writePtr, framesToRead, &framesToRead);
			if (res != MA_SUCCESS || framesToRead == 0)
				break;
			writePtr += framesToRead * bytesPerFrame;
			framesRemaining -= framesToRead;
		}

		// Restore original decoder position
		ma_decoder_seek_to_pcm_frame(decoder, originalPos);

		if (framesRemaining != 0)
			return {};   // incomplete read

		// Compute SHA-256 hash using the existing Buffer + sha256 function
		BitPouncePack::Buffer buffer;
		buffer.Data = result.Data.get();   // non-owning view
		buffer.Size = totalBytes;
		result.hash = BitPouncePack::sha256(buffer);

		// Prevent Buffer destructor from freeing our memory (which was allocated with new[])
		buffer.Data = nullptr;
		buffer.Size = 0;

		return result;
	}
}