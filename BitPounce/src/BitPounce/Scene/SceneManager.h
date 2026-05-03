#pragma once
#include "Scene.h"


namespace BitPounce
{
	class SceneManager
	{
	public:
		SceneManager();
		virtual ~SceneManager() = default;

		virtual void LoadScene(std::string name);
		virtual void AddScene(Ref<Scene> scene);
		virtual void AddAssetMap(AssetMap assetMap);
		virtual Ref<Scene> GetScene() { return m_CurrScene; }
	protected:
		std::vector<Ref<Scene>> m_Scenes = {};
		Ref<Scene> m_CurrScene = {};
	};
}