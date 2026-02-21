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
    // ---------- Constants ----------
    constexpr size_t MAX_COMMANDS = 64;
    constexpr float  MAX_VOLUME   = 1.0f;
    constexpr ma_uint32 PERIOD_SIZE_FRAMES = 1024;
    constexpr ma_uint32 PERIOD_COUNT       = 2;

    // ---------- Audio Source ----------
    struct AudioSource
    {
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
        ma_device device;
        bool initialized = false;
        ma_uint32 maxFramesPerCallback = PERIOD_SIZE_FRAMES;

        std::mutex sourceMutex;
        std::vector<AudioSource*> sources;   // raw pointers – manual management

        CommandQueue cmdQueue;
    };

    static AudioDeviceData* g_Data = nullptr;

    static inline float SoftClip(float x)
    {
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        return x * (1.5f - 0.5f * x * x);
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
        }

        data->sourceMutex.unlock();
    }

    void AudioDevice::Init()
    {
        if (g_Data != nullptr)
            return;

        auto* data = new AudioDeviceData();

        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format   = ma_format_f32;
        config.playback.channels = 0;               // stereo
        config.sampleRate        = 0;                // 48 kHz
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

        g_Data = data;
    }

    void AudioDevice::UnInit()
    {
        if (!g_Data)
            return;

        ma_device_uninit(&g_Data->device);

        // Manually delete all sources
        {
            std::lock_guard<std::mutex> lock(g_Data->sourceMutex);
            for (auto* src : g_Data->sources)
                delete src;
            g_Data->sources.clear();
        }

        delete g_Data;
        g_Data = nullptr;
    }

    AudioID AudioDevice::Load(const std::filesystem::path& filepath, bool loop)
    {
        if (!g_Data || !g_Data->initialized)
            return 0;

        auto* source = new AudioSource();   // raw allocation

        if (ma_decoder_init_file(filepath.string().c_str(), nullptr, &source->decoder) != MA_SUCCESS)
        {
            delete source;
            return 0;
        }

        ma_data_converter_config converterConfig =
            ma_data_converter_config_init(
                source->decoder.outputFormat,
                ma_format_f32,
                source->decoder.outputChannels,
                g_Data->device.playback.channels,
                source->decoder.outputSampleRate,
                g_Data->device.sampleRate
            );
        if (ma_data_converter_init(&converterConfig, nullptr, &source->converter) != MA_SUCCESS)
        {
            ma_decoder_uninit(&source->decoder);
            delete source;
            return 0;
        }

        if (!source->allocateBuffers(
                g_Data->maxFramesPerCallback,
                g_Data->device.playback.channels,
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
            std::lock_guard<std::mutex> lock(g_Data->sourceMutex);
            index = g_Data->sources.size();
            g_Data->sources.push_back(source);
        }

        return static_cast<AudioID>(index + 1);
    }

    void AudioDevice::Play(AudioID id)
    {
        if (!g_Data || !g_Data->initialized || id == 0)
            return;

        CommandQueue::Command cmd;
        cmd.type = CommandQueue::Type::Play;
        cmd.handle = id;
        g_Data->cmdQueue.push(cmd);
    }

    void AudioDevice::Stop(AudioID id)
    {
        if (!g_Data || !g_Data->initialized || id == 0)
            return;

        CommandQueue::Command cmd;
        cmd.type = CommandQueue::Type::Stop;
        cmd.handle = id;
        g_Data->cmdQueue.push(cmd);
    }

    void AudioDevice::SetVolume(AudioID id, float volume)
    {
        if (!g_Data || !g_Data->initialized || id == 0)
            return;

        CommandQueue::Command cmd;
        cmd.type = CommandQueue::Type::SetVolume;
        cmd.handle = id;
        cmd.value = volume;
        g_Data->cmdQueue.push(cmd);
    }

    void AudioDevice::SetLooping(AudioID id, bool loop)
    {
        if (!g_Data || !g_Data->initialized || id == 0)
            return;

        CommandQueue::Command cmd;
        cmd.type = CommandQueue::Type::SetLooping;
        cmd.handle = id;
        cmd.flag = loop;
        g_Data->cmdQueue.push(cmd);
    }
    bool AudioDevice::UnLoad(AudioID id)
    {

        if (!g_Data || !g_Data->initialized || id == 0)
            return false;

        size_t idx = static_cast<size_t>(id - 1);

        std::lock_guard<std::mutex> lock(g_Data->sourceMutex);

        if (idx >= g_Data->sources.size())
            return false;

        AudioSource* src = g_Data->sources[idx];
        if (!src)
            return false;   // already unloaded

        delete src;
        g_Data->sources[idx] = nullptr;   // leave a hole
        return true;
    }
}