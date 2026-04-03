#pragma once

#include <filesystem>
#include "Panel.h"
#include <BitPounce.h>

namespace BitPounce {

	class ContentBrowserPanel: public Panel
	{
	public:
		ContentBrowserPanel();

		void SetBaseDir(std::filesystem::path path);

		virtual void OnImGuiDraw() override;
	private:
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_BaseDirectory;
		
        Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

        std::unordered_map<std::filesystem::path, Ref<Texture2D>> m_Icons;
	};

}