#include <bp_pch.h>
#include "SceneImporter.h"
#include <BitPounce/Scene/SceneSerializer.h>

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

}