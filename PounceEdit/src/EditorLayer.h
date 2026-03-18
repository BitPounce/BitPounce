#pragma once

#include "BitPounce.h"
#include "Panels/Panel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

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
		bool OnKeyPressed(KeyPressedEvent& e);

		void OnNewScene(Ref<Scene> scene);
		void NewScene();
		void OpenScene(const std::filesystem::path& path);
		void OpenScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();

		// UI Panels
		void UI_Toolbar();
	private:
		EditorCamera m_EditorCamera;

		PanelManager m_Panels;
		OrthographicCameraController m_CameraController;
		float m_time = 0.0f;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_HoveredEntity;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		Ref<Texture2D> m_PlayerTexture;
		Ref<Texture2D> m_Icon;
		Ref<Texture2D> m_CheckerboardTexture;
		Ref<Framebuffer> m_Framebuffer;

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		int m_GizmoType = 7;
		glm::vec2 m_RendorSize = {};
		glm::vec2 m_ViewportBounds[2];

		SceneHierarchyPanel* m_SceneHierarchyPanel;
		ContentBrowserPanel* m_ContentBrowserPanel;

		enum class SceneState
		{
			Edit = 0, Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconStop;
	};

}