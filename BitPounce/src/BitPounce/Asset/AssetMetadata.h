#pragma once

#include "Asset.h"

#include <filesystem>
#include <optional>

namespace BitPounce {

	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
        std::optional<void*> data;
        

		operator bool() const { return Type != AssetType::None; }
	};

}