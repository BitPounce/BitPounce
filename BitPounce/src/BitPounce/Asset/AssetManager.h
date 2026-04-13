#pragma once

#include "AssetManagerBase.h"

#include "BitPounce/Project/Project.h"

namespace BitPounce {

	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::dynamic_pointer_cast<T>(asset);
		}
	};
}