#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include <BitPouncePack/BitPouncePack.h>

#include <BitPounce/Audio/Audio.h>

namespace BitPounce
{
	class AudioImporter
	{
	public:
		static Ref<Audio> ImportAudio(AssetHandle handle, const AssetMetadata& metadata);
		static bool ExportAudio(AssetHandle handle, const AssetMetadata& metadata, BitPouncePack::Pack& pack);
		static Ref<Audio> LoadAsset(BitPouncePack::PackAsset &packAsset);
	};
}