#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include <BitPouncePack/BitPouncePack.h>

#include "BitPounce/Renderer/Texture.h"

namespace BitPounce {

	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> ImportTexture2D(const std::filesystem::path& filepath);
		static Buffer LoadTextureData(const std::filesystem::path& filepath, int* width, int* height, int* channels);
		static bool ExportAsset(AssetHandle handle, const AssetMetadata& metadata, BitPouncePack::Pack& pack);
		static Ref<Texture2D> LoadAsset(BitPouncePack::PackAsset& packAsset);
	};



}