#include <bp_pch.h>

#include "AudioImporter.h"
#include "AssetManager.h"

namespace BitPounce
{
	Ref<Audio> AudioImporter::ImportAudio(AssetHandle handle, const AssetMetadata &metadata)
	{
		// just call Audio::Create
		Ref<Audio> audio = Audio::Create(metadata.FilePath);
		audio->Handle = handle;
		return audio;
	}
	bool AudioImporter::ExportAudio(AssetHandle handle, const AssetMetadata &metadata, BitPouncePack::Pack &pack)
	{
		Ref<Audio> asset = AssetManager::GetAsset<Audio>(handle);
		BitPouncePack::PackAudio packAudio = AudioDevice::AudioToPackAudio(asset->GetID(), handle);
		pack.assets.push_back(BitPouncePack::PackAsset{ std::move(packAudio) });
		return true;
	}
	Ref<Audio> AudioImporter::LoadAsset(BitPouncePack::PackAsset &packAsset)
	{
		const auto a = std::get_if<BitPouncePack::PackAudio>(&packAsset);
		return Audio::Create(*a);
	}
}