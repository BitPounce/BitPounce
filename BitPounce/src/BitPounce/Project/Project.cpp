#include "bp_pch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace BitPounce
{
    Ref<Project> Project::New(const ProjectConfig& projectConfig)
	{
		s_ActiveProject = CreateRef<Project>(projectConfig);
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path, bool isEditor)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			if(isEditor)
			{
				std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
				s_ActiveProject->m_AssetManager = editorAssetManager;
				editorAssetManager->DeserializeAssetRegistry();
			}
			else
			{
				std::shared_ptr<RuntimeAssetManager> runtimeAssetManager = std::make_shared<RuntimeAssetManager>();
				for(auto&& assetPack : s_ActiveProject->m_Config.AssetPacks)
				{
					BP_CORE_INFO("{}" ,assetPack.string());
					runtimeAssetManager->LoadAssetPack(assetPack);
				}
				s_ActiveProject->m_AssetManager = runtimeAssetManager;
				s_ActiveProject->m_SceneManager.AddAssetMap(s_ActiveProject->GetRuntimeAssetManager()->GetAssetMap());
				s_ActiveProject->m_SceneManager.LoadScene(s_ActiveProject->GetConfig().StartScene.filename().generic_string());
			}
			
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}

		return false;
	}
}