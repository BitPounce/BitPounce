#include <bp_pch.h>
#include "Audio.h"

namespace BitPounce
{
	Ref<Audio> Audio::Create(const std::filesystem::path& filepath, bool loop)
	{
		return CreateRef<Audio>(filepath, loop);
	}

	Ref<Audio> Audio::Create(const BitPouncePack::PackAudio &packAudio, bool loop)
	{
		return CreateRef<Audio>(packAudio, loop);
	}

	void Audio::Play()
	{
		AudioDevice::Play(id);
	}

	void Audio::Stop()
	{
		AudioDevice::Stop(id);
	}

	void Audio::SetVolume(float volume)
	{
		AudioDevice::SetVolume(id, volume);
	}

	void Audio::SetLooping(bool loop)
	{
		AudioDevice::SetLooping(id, loop);
	}

	AudioID Audio::GetID()
	{
		return id;
	}

	Audio::Audio(const std::filesystem::path& filepath, bool loop)
	{
		id = AudioDevice::Load(filepath, loop);
	}

	Audio::Audio(const BitPouncePack::PackAudio &packAudio, bool loop)
	{
		id = AudioDevice::Load(packAudio, loop);
		this->Handle = packAudio.AssetHandle;
	}

	Audio::~Audio()
	{
		AudioDevice::UnLoad(id);
	}
}
