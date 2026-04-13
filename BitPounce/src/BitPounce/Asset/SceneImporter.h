#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "BitPounce/Scene/Scene.h"

namespace BitPounce {

	class SceneImporter
	{
	public:
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);
	};



}