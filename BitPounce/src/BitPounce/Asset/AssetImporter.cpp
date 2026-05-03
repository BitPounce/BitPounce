#include "bp_pch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "AudioImporter.h"
#include "FontImporter.h"
#include "SceneImporter.h"
#include "BitPounce/Core/FileSystem.h"

#include <map>

static BitPounce::AssetType GetAssetTypeForPackAsset(const BitPouncePack::PackAsset& asset)
{
	if (const auto* f = std::get_if<BitPouncePack::PackFile>(&asset)) {
		constexpr uint64_t KiB = 1024;
		constexpr uint64_t MiB = KiB * 1024;
		constexpr uint64_t GiB = MiB * 1024;
		if(f->Size >= GiB)
		{
			BitPounce::Buffer buffer = BitPounce::Buffer(f->Size);
			std::memcpy(buffer.As<std::byte>(), f->Data.get(), f->Size);
			// Wait a minute... ITS A FAKE FILE RUN!!!!!
			BitPounce::FileSystem::AddFakeFile(f->Name, buffer.ToBuffer());
		}
		else
		{
			BitPounce::Buffer buffer = BitPounce::Buffer(f->Size);
			std::memcpy(buffer.As<std::byte>(), f->Data.get(), f->Size);
			// Wait a minute... ITS A FAKE FILE RUN!!!!!
			BitPounce::FileSystem::AddFakeFile(f->Name, BitPounce::Buffer::Copy(buffer));
		}
		
	}
    if (const auto* t = std::get_if<BitPouncePack::PackTexture>(&asset)) { BP_CORE_INFO("PackTexture {}", t->AssetHandle); return BitPounce::AssetType::Texture2D;}
    if (const auto* s = std::get_if<BitPouncePack::PackScene>(&asset)) { BP_CORE_INFO("PackScene {}", s->AssetHandle); return BitPounce::AssetType::Scene;}
    if (const auto* a = std::get_if<BitPouncePack::PackAudio>(&asset)) { BP_CORE_INFO("PackAudio {}", a->AssetHandle); return BitPounce::AssetType::Audio;}
    if (const auto* f = std::get_if<BitPouncePack::PackFont>(&asset)) { BP_CORE_INFO("PackFont {}", f->AssetHandle); return BitPounce::AssetType::Font;}
    return BitPounce::AssetType::None;
}

namespace BitPounce {

	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	using AssetExportFunction = std::function<bool(AssetHandle, const AssetMetadata&, BitPouncePack::Pack&)>;
	using AssetLoadFunction = std::function<Ref<Asset>(BitPouncePack::PackAsset&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Texture2D, [](UUID handle, const AssetMetadata& metadata) {
        	return TextureImporter::ImportTexture2D(handle, metadata);
    	}},
		{ AssetType::Audio, AudioImporter::ImportAudio },
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::Scene, SceneImporter::ImportScene }
	};
	static std::map<AssetType, AssetExportFunction> s_AssetExportFunction = {
		{AssetType::Texture2D, TextureImporter::ExportAsset},
		{AssetType::Scene, SceneImporter::ExportScene},
		{AssetType::Audio, AudioImporter::ExportAudio},
		{AssetType::Font, FontImporter::ExportFont}
	};
	static std::map<AssetType, AssetLoadFunction> s_AssetLoadFunction = {
		{AssetType::Texture2D, TextureImporter::LoadAsset},
		{AssetType::Scene, SceneImporter::LoadAsset},
		{AssetType::Audio, AudioImporter::LoadAsset},
		{AssetType::Font, FontImporter::LoadAsset}
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

    bool AssetImporter::ExportAsset(AssetHandle handle, const AssetMetadata &metadata, BitPouncePack::Pack &pack)
    {
		if (s_AssetExportFunction.find(metadata.Type) == s_AssetExportFunction.end())
		{
			BP_CORE_ERROR("No exporter available for asset type: {}", (uint16_t)metadata.Type);
			return false;
		}

        return s_AssetExportFunction.at(metadata.Type)(handle, metadata, pack);
    }

    Ref<Asset> AssetImporter::LoadAsset(BitPouncePack::PackAsset &asset)
    {
		AssetType Type = GetAssetTypeForPackAsset(asset);
        if (s_AssetLoadFunction.find(Type) == s_AssetLoadFunction.end())
		{
			return nullptr;
		}

		return s_AssetLoadFunction.at(Type)(asset);
    }
}