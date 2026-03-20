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

    void SceneHierarchyPanel::SetContext(const Ref<Scene> scene)
    {
		m_Context.reset();
        m_Context = scene;
		m_SelectionContext = {};
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
			for(auto&& c : entity.GetChildren())
			{
				auto& tagc = entity.GetComponent<TagComponent>();
				DrawEntityNode(c, tagc);
			}

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

        auto view = m_Context->GetRegistry((*(ECSSystem*)(0))).view<TagComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& tagc = view.get<TagComponent>(entity);
			auto& transformComponent = view.get<TransformComponent>(entity);

			if(!transformComponent.Parent)
				DrawEntityNode(Entity{entity, m_Context.get()}, tagc);
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				auto&& e = m_Context->CreateEntity("Empty Entity");
				if(m_SelectionContext)
				{
					e.SetParent(m_SelectionContext);
				}
			}
			

			ImGui::EndPopup();
		}

        ImGui::End();

        ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);

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