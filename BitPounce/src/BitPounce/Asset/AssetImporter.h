#pragma once

#include "AssetMetadata.h"
#include <BitPouncePack/BitPouncePack.h>

namespace BitPounce {

	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
		static bool ExportAsset(AssetHandle handle, const AssetMetadata& metadata, BitPouncePack::Pack& pack);
		static Ref<Asset> LoadAsset(BitPouncePack::PackAsset& asset);
	};

}