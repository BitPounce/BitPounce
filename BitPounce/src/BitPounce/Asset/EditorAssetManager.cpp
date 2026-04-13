#include "bp_pch.h"
#include "AssetManager.h"
#include "EditorAssetManager.h"
#include "AssetImporter.h"
#include <nlohmann/json.hpp>
#include <BitPounce/Events/AssetManagerEvent.h>
#include <BitPounce/Core/Application.h>

namespace BitPounce 
{
	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end() && m_LoadedAssets[handle] && m_LoadedAssets[handle].get();
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path &filepath)
	{
		for (const auto&[handle, metadata] : m_AssetRegistry)
		{
			if(metadata.FilePath == filepath)
			{
				return handle;
			}
		}

		AssetHandle handle = AssetHandle(); // new handle time!!!
		
		return ImportAsset(filepath, handle);
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath, AssetHandle handle)
	{
		AssetMetadata metadata = AssetMetadata();
		metadata.FilePath = filepath;

		metadata.Type = Asset::GetTypeFromFileExtension(filepath.extension());
		{
			AssetPreLoadedEvent preloadedEvent = AssetPreLoadedEvent(metadata, handle);
			Application::Get().OnEvent(preloadedEvent);
		}
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		asset->Handle = handle;

		metadata.Type = Asset::GetTypeFromFileExtension(filepath.extension());
		{
			AssetLoadedEvent loadedEvent = AssetLoadedEvent(metadata, handle);
			Application::Get().OnEvent(loadedEvent);
		}
		if (asset)
		{
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
			return handle;
		}
		return 0;
	}

	void EditorAssetManager::SerializeAssetRegistry()
	{
		auto path = Project::GetAssetRegistryPath();
		auto&& assetDir = Project::GetAssetDirectory();

		nlohmann::json out;

		{
			nlohmann::json assets = out.array();

			for (const auto&[handle, metadata] : m_AssetRegistry)
			{
				nlohmann::json asset = nlohmann::json();

				asset["Handle"] = handle.operator std::size_t();
				std::string filepathStr = std::filesystem::relative(metadata.FilePath, assetDir).generic_string();
				asset["FilePath"] =  filepathStr;
				asset["Type"] = AssetTypeToString(metadata.Type);

				assets.push_back(asset);
				
			}

			out["AssetRegistry"] = assets;
		}
		std::ofstream fout(path);
		fout << out.dump(1, '\t');
	}

	bool EditorAssetManager::DeserializeAssetRegistry()
	{
		auto path = Project::GetAssetRegistryPath();
		auto&& assetDir = Project::GetAssetDirectory();

		if (!std::filesystem::exists(path))
			return false;

		std::ifstream fin(path);
		if (!fin.is_open())
			return false;

		nlohmann::json data;
		fin >> data;

		if (!data.contains("AssetRegistry"))
			return false;

		auto& assets = data["AssetRegistry"];

		m_AssetRegistry.clear();

		for (auto& asset : assets)
		{
			AssetHandle handle = asset["Handle"].get<std::size_t>();

			AssetMetadata metadata;
			metadata.FilePath = assetDir / asset["FilePath"].get<std::string>();
			metadata.Type = AssetTypeFromString(asset["Type"].get<std::string>());

			m_AssetRegistry[handle] = metadata;
		}

		return true;
	
	}

	void EditorAssetManager::ReimportAsset(AssetHandle handle)
	{
		AssetMetadata& metadata = m_AssetRegistry[handle];
		m_LoadedAssets.erase(handle);

		ImportAsset(metadata.FilePath, handle);
		m_LoadedAssets[handle] = LoadAsset(handle);

	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		static AssetMetadata s_NullMetadata;
		auto it = m_AssetRegistry.find(handle);
		if (it == m_AssetRegistry.end())
			return s_NullMetadata;

		return it->second;
	}

	Ref<Asset> EditorAssetManager::LoadAsset(AssetHandle handle) const
	{
		Ref<Asset> asset;

		// this is an ugly, disgusting hack. FIX ME!!!!!!!!!!!
		const AssetMetadata& temp = GetMetadata(handle);
		AssetMetadata& metadata = *(AssetMetadata*)&temp;

		{
			AssetPreLoadedEvent preloadedEvent = AssetPreLoadedEvent(metadata, handle);
			Application::Get().OnEvent(preloadedEvent);
		}

		asset = AssetImporter::ImportAsset(handle, metadata);

		{
			AssetLoadedEvent loadedEvent = AssetLoadedEvent(metadata, handle);
			Application::Get().OnEvent(loadedEvent);
		}
		if (!asset)
		{
			// import failed
			BP_CORE_ERROR("EditorAssetManager::GetAsset - asset import failed!");
		}

		m_LoadedAssets[handle] = asset;

		return asset;
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		// 1. check if handle is valid
		if (!IsAssetHandleValid(handle))
			return nullptr;

		// 2. check if asset needs load (and if so, load)
		Ref<Asset> asset;
		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			asset = LoadAsset(handle);
		}
		// 3. return asset
		return asset;
	}

}