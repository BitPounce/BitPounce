#pragma once

#include "BitPounce.h"
#include "Panels/Panel.h"
#include "Panels/SceneHierarchyPanel.h"

namespace BitPounce {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep& ts) override;
		virtual void OnImGuiRender() override;
		void OnDockSpace();
		void OnEvent(Event& e) override;


	private:
		PanelManager m_Panels;
		OrthographicCameraController m_CameraController;
		float m_time = 0.0f;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		Ref<Texture2D> m_PlayerTexture;
		Ref<Texture2D> m_Icon;
		Ref<Texture2D> m_CheckerboardTexture;
		Ref<Framebuffer> m_Framebuffer;

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		glm::vec2 m_RendorSize = {};
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}