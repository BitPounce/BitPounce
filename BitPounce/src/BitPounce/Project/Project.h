#pragma once

#include <string>
#include <filesystem>

#include "BitPounce/Core/Base.h"
#include "BitPounce/Core/Logger.h"

#include "BitPounce/Asset/RuntimeAssetManager.h"
#include "BitPounce/Asset/EditorAssetManager.h"

namespace BitPounce 
{

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;
		// Relative to AssetDirectory
		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath = "AssetRegistry.bpreg";
	};

	class Project
	{
	public:
		Project(const ProjectConfig& projectConfig = ProjectConfig()) : m_Config(projectConfig)
		{

		}
		static const std::filesystem::path& GetProjectDirectory()
		{
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetRegistryPath()
		{
			BP_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		// TODO: move to asset manager when we have one
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			return GetAssetDirectory() / path;
		}

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		std::shared_ptr<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
		std::shared_ptr<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }
		std::shared_ptr<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }

		static Ref<Project> New(const ProjectConfig& projectConfig = ProjectConfig());
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		std::shared_ptr<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};

}