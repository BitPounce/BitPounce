#include "bp_pch.h"
#include "AssetManager.h"
#include "RuntimeAssetManager.h"
#include <BitPouncePack/BitPouncePack.h>
#include "AssetImporter.h"

namespace BitPounce 
{


	Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle) const
	{
		auto it = m_LoadedAssets.find(handle);
		if (it == m_LoadedAssets.end())
		{
		    BP_CORE_ERROR("Asset not found: {}", handle.operator std::uint64_t());
			BP_CORE_ASSERT(0, "Asset not found: {}", handle.operator std::uint64_t());
		    return nullptr;
		}
		return it->second;
	}

	bool RuntimeAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return handle != 0 && IsAssetLoaded(handle);
	}

	bool RuntimeAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end() && m_LoadedAssets[handle] && m_LoadedAssets[handle].get();
	}

	void RuntimeAssetManager::LoadAssetPack(const std::filesystem::path &path)
	{
		BitPouncePack::Pack pack = BitPouncePack::Load(Project::GetAssetFileSystemPath(path));
		
		for (auto&& asset : pack.assets)
		{
			Ref<Asset> loadedAsset = AssetImporter::LoadAsset(asset);
			if(loadedAsset)
			{
				m_LoadedAssets[loadedAsset->Handle] = loadedAsset;
			}

		}

		for (const auto& [handle, asset] : m_LoadedAssets)
		{
		    BP_CORE_INFO("Loaded Asset: {}", handle.operator std::uint64_t());
		}
		
	}
}
