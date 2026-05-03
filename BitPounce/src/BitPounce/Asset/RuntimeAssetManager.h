#pragma once

#include "AssetManagerBase.h"

namespace BitPounce {

	class RuntimeAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual void LoadAssetPack(const std::filesystem::path& path);

		virtual AssetMap& GetAssetMap() { return m_LoadedAssets; }
	private:
		//AssetRegistry m_AssetRegistry;
		mutable AssetMap m_LoadedAssets;
	};
}