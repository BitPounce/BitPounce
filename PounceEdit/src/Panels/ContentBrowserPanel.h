#pragma once

#include <filesystem>
#include "Panel.h"

namespace BitPounce {

	class ContentBrowserPanel: public Panel
	{
	public:
		ContentBrowserPanel();

		virtual void OnImGuiDraw() override;
	private:
		std::filesystem::path m_CurrentDirectory;
	};

}