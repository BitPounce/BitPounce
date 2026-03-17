#include "bp_pch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace BitPounce {

	// Once we have projects, change this
	extern const std::filesystem::path g_AssetPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_name = "Content Browser Panel";
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/Flor.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/File.png");
	}

	void ContentBrowserPanel::OnImGuiDraw()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			std::string filenameString = relativePath.filename().string();

			std::string id = "##icon_" + path.string();

			Ref<Texture2D> icon = nullptr;
			if (directoryEntry.is_directory())
			{
				icon = m_DirectoryIcon;
			}
			else
			{
				auto it = m_Icons.find(relativePath);
				if (it != m_Icons.end())
				{
					icon = it->second;
				}
				else if (relativePath.extension() == ".png") // make sure to include the dot
				{
					icon = Texture2D::Create(path.string());
					m_Icons[relativePath] = icon;
				}
				else
				{
					icon = m_FileIcon;
				}
			}
			ImGui::PushID(filenameString.c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(id.c_str(), (ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				const std::wstring itemPath = relativePath.generic_wstring();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), ((itemPath.size()) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();

			}
			ImGui::PopStyleColor();
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();

		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		// TODO: status bar
		ImGui::End();
	}

}