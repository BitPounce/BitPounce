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

		static Ref<Audio> Create(std::filesystem::path filepath);
		~Audio();
		Audio(std::filesystem::path filepath);
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