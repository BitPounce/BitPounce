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
	
	class Asset
	{
	public:
		AssetHandle Handle; // Generate handle

		virtual AssetType GetType() const = 0;
	};
}