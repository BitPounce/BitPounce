#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "BitPounce/Renderer/Texture.h"

namespace BitPounce {

	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
	};



}