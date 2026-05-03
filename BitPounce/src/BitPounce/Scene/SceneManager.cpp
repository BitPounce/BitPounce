#include <bp_pch.h>
#include "SceneManager.h"
#include <BitPounce/Core/Application.h>

namespace BitPounce
{
	SceneManager::SceneManager()
	{
	}

	void SceneManager::LoadScene(std::string name)
	{
		if(m_CurrScene) { m_CurrScene->OnRuntimeStop(); }
		for(auto&& scene : m_Scenes)
		{
			if(scene->name == name)
			{
				m_CurrScene = Scene::Copy(scene);
				m_CurrScene->OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
				m_CurrScene->OnRuntimeStart();
			}
		}
	}

	void SceneManager::AddScene(Ref<Scene> scene)
	{
		m_Scenes.push_back(Scene::Copy(scene));
	}

	void SceneManager::AddAssetMap(AssetMap assetMap)
	{
		for (auto&& assetPair : assetMap)
		{
			if(assetPair.second->GetType() == BitPounce::AssetType::Scene)
			{
				AddScene(std::dynamic_pointer_cast<BitPounce::Scene>(assetPair.second));
			}
		}
		
	}
}