#include "SceneHierarchyPanel.h"

#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

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
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>().Transform;
				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::TreePop();
			}
		}

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

        ImGui::End();

        ImGui::Begin("Properties");
		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
    }
}