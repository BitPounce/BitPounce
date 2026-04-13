#pragma once

#include "Asset.h"

#include <filesystem>
#include <optional>
#include <BitPounce/Scene/ECSSystem.h>


namespace BitPounce 
{
	struct SceneAssetMetadata
	{
		std::vector<Ref<ECSSystem>> Systems;
	};

	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
        std::optional<void*> data;
        

		operator bool() const { return Type != AssetType::None; }
	};

}