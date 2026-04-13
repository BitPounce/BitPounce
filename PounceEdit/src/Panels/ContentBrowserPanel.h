#pragma once

#include <filesystem>
#include "Panel.h"
#include <BitPounce.h>

namespace BitPounce {

	class ContentBrowserPanel: public Panel
	{
	public:
		ContentBrowserPanel();

		void SetBaseDir(const std::filesystem::path& path);

		virtual void OnImGuiDraw() override;
	private:
		void RefreshAssetTree();
	private:
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_BaseDirectory;
		
        Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

        std::unordered_map<std::filesystem::path, Ref<Texture2D>> m_Icons;

		struct TreeNode
		{
			std::filesystem::path Path;
			
			uint32_t Parent = (uint32_t)-1;
			std::map<std::filesystem::path, uint32_t> Children;
			AssetHandle Handle = 0;

			TreeNode(const std::filesystem::path& path)
				: Path(path) {}
		};
				
		std::vector<TreeNode> m_TreeNodes;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_AssetTree;

		enum class Mode
		{
			Asset = 0, FileSystem = 1
		};

		Mode m_Mode = Mode::Asset;
	};

}