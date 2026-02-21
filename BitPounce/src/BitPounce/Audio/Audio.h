#include "AudioDevice.h"
#include "BitPounce/Core/Base.h"

namespace BitPounce
{
    class Audio
    {
    public:
        Audio(const Audio&) = delete;
        Audio& operator=(const Audio&) = delete;

        Audio(Audio&&) = default;
        Audio& operator=(Audio&&) = default;

        static Ref<Audio> Create(std::filesystem::path filepath);
        ~Audio();
        Audio(std::filesystem::path filepath);
        void Play();
        void Stop();
        void SetVolume(float volume);
        void SetLooping(bool loop);

        AudioID GetID();
    private:
        
        AudioID id;

        friend class std::shared_ptr<Audio>;
    };
}