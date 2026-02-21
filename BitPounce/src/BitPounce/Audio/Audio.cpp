#include <bp_pch.h>
#include "Audio.h"

namespace BitPounce
{
    Ref<Audio> Audio::Create(std::filesystem::path filepath)
    {
        return CreateRef<Audio>(filepath);
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

    Audio::Audio(std::filesystem::path filepath)
    {
        id = AudioDevice::Load(filepath);
    }
    
    Audio::~Audio()
    {
        AudioDevice::UnLoad(id);
    }
}
