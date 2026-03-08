#include "SceneHierarchyPanel.h"

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

    void SceneHierarchyPanel::OnImGuiDraw()
    {
        ImGui::Begin("Scene Hierarchy");

        auto view = m_Context->GetRegistry((*(ECSSystem*)(0))).view<TagComponent>();
        for (auto entity : view)
        {
            auto& tc = view.get<TagComponent>(entity);
            DrawEntityNode(Entity{entity, m_Context.get()}, tc);
        }

        ImGui::End();
    }
}