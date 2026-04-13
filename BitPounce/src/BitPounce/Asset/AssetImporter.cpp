#include "bp_pch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "AudioImporter.h"
#include "FontImporter.h"
#include "SceneImporter.h"

#include <map>

namespace BitPounce {

	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Texture2D, [](UUID handle, const AssetMetadata& metadata) {
        	return TextureImporter::ImportTexture2D(handle, metadata);
    	}},
		{ AssetType::Audio, AudioImporter::ImportAudio },
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::Scene, SceneImporter::ImportScene }
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			BP_CORE_ERROR("No importer available for asset type: {}", (uint16_t)metadata.Type);
			return nullptr;
		}

		return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
	}

}