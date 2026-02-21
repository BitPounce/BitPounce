#pragma once
#include <filesystem>
#include <cstdint>

namespace BitPounce
{
    typedef uint64_t AudioID;

    class AudioDevice
    {
    public:
        static void Init();
        static AudioID Load(const std::filesystem::path& filepath, bool loop = false);
        static void UnInit();
        static void Play(AudioID id);
        static void Stop(AudioID id);
        static void SetVolume(AudioID id, float volume);
        static void SetLooping(AudioID id, bool loop);
        static bool UnLoad(AudioID id);
    };
}