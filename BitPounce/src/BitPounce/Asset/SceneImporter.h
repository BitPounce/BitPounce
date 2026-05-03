#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include <BitPouncePack/BitPouncePack.h>
#include "BitPounce/Scene/Scene.h"

namespace BitPounce {

	class SceneImporter
	{
	public:
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);
		static bool ExportScene(AssetHandle handle, const AssetMetadata& metadata, BitPouncePack::Pack& pack);
		static Ref<Scene> LoadAsset(BitPouncePack::PackAsset& packAsset);
	};



}