#include <bp_pch.h>
#include "SceneImporter.h"
#include <BitPounce/Scene/SceneSerializer.h>
#include "AssetManager.h"
#include <BitPounce/Events/AssetManagerEvent.h>
#include <BitPounce/Core/Application.h>

namespace BitPounce
{

	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata &metadata)
	{
		Ref<Scene> scene = CreateRef<Scene>(metadata.FilePath.filename().string().c_str());
		std::vector<ECSSystem*> sys = std::vector<ECSSystem*>();
		for (auto _sys : ((SceneAssetMetadata*)metadata.data.value())->Systems)
		{
			sys.push_back((ECSSystem*)_sys->clone());
		}

		for (auto _sys : sys)
		{
			scene->GetSysManager().AddSys_in(_sys);
		}
		scene->AddedAllSys();
		SceneSerializer sceneSerializer = SceneSerializer(scene);
		sceneSerializer.Deserialize(metadata.FilePath);
		
		return scene;
	}

	bool SceneImporter::ExportScene(AssetHandle handle, const AssetMetadata &metadata, BitPouncePack::Pack &pack)
	{
		Ref<Scene> scene = AssetManager::GetAsset<Scene>(handle);
		SceneSerializer sceneSerializer = SceneSerializer(scene);
		pack.assets.push_back(sceneSerializer.SerializeRuntime());
		return true;
	}
    Ref<Scene> SceneImporter::LoadAsset(BitPouncePack::PackAsset &packAsset)
    {
		auto s = std::get_if<BitPouncePack::PackScene>(&packAsset);
		AssetMetadata metadata = {};
		metadata.Type = AssetType::Scene;
		metadata.FilePath = s->Name;

		AssetPreLoadedEvent preloadedEvent = AssetPreLoadedEvent(metadata, s->AssetHandle);
		Application::Get().OnEvent(preloadedEvent);

		Ref<Scene> scene = CreateRef<Scene>(metadata.FilePath.filename().string().c_str());
		std::vector<ECSSystem*> sys = std::vector<ECSSystem*>();
		for (auto _sys : ((SceneAssetMetadata*)metadata.data.value())->Systems)
		{
			sys.push_back((ECSSystem*)_sys->clone());
		}

		for (auto _sys : sys)
		{
			scene->GetSysManager().AddSys_in(_sys);
		}
		scene->AddedAllSys();
		SceneSerializer sceneSerializer = SceneSerializer(scene);
		sceneSerializer.DeserializeRuntime(*s);

        return scene;
    }
}