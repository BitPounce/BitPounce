#pragma once

#include <filesystem>
#include "Panel.h"
#include <BitPounce.h>

namespace BitPounce {

	class ContentBrowserPanel: public Panel
	{
	public:
		ContentBrowserPanel();

		virtual void OnImGuiDraw() override;
	private:
		std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

        std::unordered_map<std::filesystem::path, Ref<Texture2D>> m_Icons;
	};

}