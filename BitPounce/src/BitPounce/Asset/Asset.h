#pragma once

#include "BitPounce/Core/UUID.h"

namespace BitPounce {

	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Texture2D,
        // TODO
        Font,
        Audio,
        Scene
	};

    std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view assetType);
	
	class Asset
	{
	public:
		AssetHandle Handle; // Generate handle

		virtual AssetType GetType() const = 0;
		static AssetType GetTypeFromFileExtension(const std::string& ext);
	};
}