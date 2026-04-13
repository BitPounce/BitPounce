#include "bp_pch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>
#include <filesystem>

namespace BitPounce {

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory("./"), m_CurrentDirectory("./")
	{
		m_TreeNodes.emplace_back("."); // root node
		m_Mode = Mode::FileSystem;

		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/Flor.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/File.png");
	}

	void ContentBrowserPanel::SetBaseDir(const std::filesystem::path& path)
	{
		m_BaseDirectory = path;
		m_CurrentDirectory = m_BaseDirectory;

		RefreshAssetTree();
		m_Mode = Mode::FileSystem;
	}

	void ContentBrowserPanel::OnImGuiDraw()
	{
		ImGui::Begin("Content Browser");

		const char* label = m_Mode == Mode::Asset ? "Asset" : "File";
		if (ImGui::Button(label))
			m_Mode = m_Mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;

		if (m_CurrentDirectory != m_BaseDirectory)
		{
			ImGui::SameLine();
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				m_Icons.clear();
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

		if (m_Mode == Mode::Asset)
		{
			TreeNode* node = &m_TreeNodes[0];
			std::filesystem::path accumulatedPath = Project::GetAssetDirectory();

			auto base = Project::GetAssetDirectory();
			std::filesystem::path relativeDir;

			if (m_CurrentDirectory != base)
				relativeDir = std::filesystem::relative(m_CurrentDirectory, base);

			if (!relativeDir.empty())
			{
				for (const auto& segment : relativeDir)
				{
					auto it = node->Children.find(segment);
					if (it == node->Children.end())
						break;

					node = &m_TreeNodes[it->second];
					accumulatedPath /= segment;
				}
			}

			for (const auto& [childPath, treeNodeIndex] : node->Children)
			{
				std::filesystem::path fullPath = accumulatedPath / childPath;
				bool isDirectory = std::filesystem::is_directory(fullPath);

				ImGui::PushID(childPath.string().c_str());

				Ref<Texture2D> icon = isDirectory ? m_DirectoryIcon : m_FileIcon;

				if (!isDirectory)
				{
				    if (Asset::GetTypeFromFileExtension(childPath.extension()) == AssetType::Texture2D)
				    {
						auto tex = AssetManager::GetAsset<Texture2D>(m_TreeNodes[treeNodeIndex].Handle);
						icon = tex;
				    }
					else if (Asset::GetTypeFromFileExtension(childPath.extension()) == AssetType::Font)
					{
						auto font = AssetManager::GetAsset<Font>(m_TreeNodes[treeNodeIndex].Handle);
						icon = font->GetAtlasTexture();
					}
					
				}
				

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				if(ImGui::ImageButton("##icon", (ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0,1 }, { 1,0 }) && Asset::GetTypeFromFileExtension(childPath.extension()) == AssetType::Audio)
				{
					auto audio = AssetManager::GetAsset<Audio>(m_TreeNodes[treeNodeIndex].Handle) ;
					audio->Play();
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import"))
						Project::GetActive()->GetEditorAssetManager()->ImportAsset(fullPath);
					ImGui::EndPopup();
				}

				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (isDirectory)
					{
						m_CurrentDirectory /= childPath;
						m_Icons.clear();
					}
				}

				ImGui::TextWrapped(childPath.string().c_str());
				ImGui::NextColumn();

				ImGui::PopID();
			}
		}
		else // FileSystem mode
		{
			for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				auto& path = entry.path();
				auto relativePath = std::filesystem::relative(path, m_BaseDirectory);

				std::string id = "##icon_" + path.string();

				Ref<Texture2D> icon = nullptr;

				if (entry.is_directory())
				{
					icon = m_DirectoryIcon;
				}
				else
				{
					auto it = m_Icons.find(path);
					if (it != m_Icons.end())
					{
						icon = it->second;
					}
					else if (path.extension() == ".png")
					{
						m_Icons[path] = Texture2D::Create(path.string());
						icon = m_Icons[path];
					}
					else
					{
						icon = m_FileIcon;
					}
				}

				ImGui::PushID(relativePath.string().c_str());

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton(id.c_str(), (ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0,1 }, { 1,0 });

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import"))
						Project::GetActive()->GetEditorAssetManager()->ImportAsset(path);
					ImGui::EndPopup();
				}

				if (ImGui::BeginDragDropSource())
				{
					const std::wstring itemPath = path.generic_wstring();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), (itemPath.size() + 1) * sizeof(wchar_t));
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (entry.is_directory())
					{
						m_CurrentDirectory /= path.filename();
						m_Icons.clear();
					}
				}

				ImGui::PopStyleColor();

				ImGui::TextWrapped(relativePath.filename().string().c_str());
				ImGui::NextColumn();

				ImGui::PopID();
			}
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16.0f, 512.0f);
		ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

		ImGui::End();
	}

	void ContentBrowserPanel::RefreshAssetTree()
	{
		m_TreeNodes.clear();
		m_TreeNodes.emplace_back("."); // root

		const auto& assetRegistry =
			Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();

		for (const auto& [handle, metadata] : assetRegistry)
		{
			uint32_t currentNodeIndex = 0;

			for (const auto& p : std::filesystem::relative(metadata.FilePath, Project::GetAssetDirectory()))
			{
				auto& node = m_TreeNodes[currentNodeIndex];
				auto it = node.Children.find(p);

				if (it != node.Children.end())
				{
					currentNodeIndex = it->second;
				}
				else
				{
					TreeNode newNode(p);
					newNode.Parent = currentNodeIndex;
					if(!std::filesystem::is_directory(Project::GetAssetFileSystemPath(metadata.FilePath)))
					{
						newNode.Handle = handle;
					}

					m_TreeNodes.push_back(newNode);
					uint32_t newIndex = (uint32_t)m_TreeNodes.size() - 1;

					m_TreeNodes[currentNodeIndex].Children[p] = newIndex;

					currentNodeIndex = newIndex;
				}
			}
		}
	}

}