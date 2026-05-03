#include <bp_pch.h>

#include "FontImporter.h"
#include "AssetManager.h"

namespace BitPounce
{
	Ref<Font> FontImporter::ImportFont(AssetHandle handle, const AssetMetadata &metadata)
	{
		Ref<Font> asset = CreateRef<Font>(metadata.FilePath);
		asset->Handle = handle;
		return asset;
	}

	bool FontImporter::ExportFont(AssetHandle handle, const AssetMetadata &metadata, BitPouncePack::Pack &pack)
	{
		Ref<Font> asset = AssetManager::GetAsset<Font>(handle);
		BitPouncePack::PackFont packFont = asset->Export();
		packFont.AssetHandle = handle;
		pack.assets.push_back(BitPouncePack::PackAsset{ std::move(packFont) });
		return true;
	}
	Ref<Font> FontImporter::LoadAsset(BitPouncePack::PackAsset &packAsset)
	{
		const auto f = std::get_if<BitPouncePack::PackFont>(&packAsset);
		return CreateRef<Font>(*f);
	}
}