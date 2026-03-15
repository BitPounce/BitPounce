#pragma once

#include "Panel.h"
#include "BitPounce.h"

namespace BitPounce
{
    class SceneHierarchyPanel : public Panel
    {
    public:
        SceneHierarchyPanel() {};
        SceneHierarchyPanel(const Ref<Scene>& scene);

        virtual void OnImGuiDraw() override;
        void SetContext(const Ref<Scene> scene);
        Entity& GetSelectedEntity() { return m_SelectionContext; }
    private:
        void DrawEntityNode(Entity entity, TagComponent tc);
        void DrawComponents(Entity entity);
    private:
        Ref<Scene> m_Context;
        Entity m_SelectionContext;
    };
}