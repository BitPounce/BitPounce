#include "AudioDevice.h"
#include "BitPounce/Core/Base.h"
#include "BitPounce/Asset/Asset.h"

namespace BitPounce
{

	class Audio : public Asset
	{
	public:
		Audio(const Audio&) = delete;
		Audio& operator=(const Audio&) = delete;

		Audio(Audio&&) = default;
		Audio& operator=(Audio&&) = default;

		static Ref<Audio> Create(const std::filesystem::path& filepath, bool loop = false);
		static Ref<Audio> Create(const BitPouncePack::PackAudio& packAudio, bool loop = false);
		~Audio();
		Audio(const std::filesystem::path& filepath, bool loop = false);
		Audio(const BitPouncePack::PackAudio& packAudio, bool loop = false);
		void Play();
		void Stop();
		void SetVolume(float volume);
		void SetLooping(bool loop);
		

		AudioID GetID();

		virtual AssetType GetType() const { return AssetType::Audio; }
	private:
		
		AudioID id;

		friend class std::shared_ptr<Audio>;
	};
}