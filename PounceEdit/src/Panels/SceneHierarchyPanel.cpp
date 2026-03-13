#include "SceneHierarchyPanel.h"

#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include <BitPounce/ImGui/ImGuiUtils.h>

namespace BitPounce
{
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
    {
        m_name = std::string("Scene Hierarchy Panel");
        SetContext(scene);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene> &scene)
    {
        m_Context = scene;
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity, TagComponent tc)
    {
        auto& tag = tc.Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGuiUtils::DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			ImGuiUtils::DrawVec3Control("Translation", component.Translation);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			ImGuiUtils::DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);
			ImGuiUtils::DrawVec3Control("Scale", component.Scale, 1.0f);
		});

        m_Context->OnEditorPropImguiDraw(entity);
    }

    void SceneHierarchyPanel::OnImGuiDraw()
    {
        ImGui::Begin("Scene Hierarchy");

        auto view = m_Context->GetRegistry((*(ECSSystem*)(0))).view<TagComponent>();
        for (auto entity : view)
        {
            auto& tc = view.get<TagComponent>(entity);
            DrawEntityNode(Entity{entity, m_Context.get()}, tc);
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::EndPopup();
		}

        ImGui::End();

        ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);

			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				m_Context->AddComponentPopupImguiDraw(m_SelectionContext);

				ImGui::EndPopup();
			}

			ImGui::PopItemWidth();
		}
			

		ImGui::End();
    }
}