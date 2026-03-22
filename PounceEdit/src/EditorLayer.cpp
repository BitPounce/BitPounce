#include "EditorLayer.h"
#include "imgui.h"
#include "BitPounce.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"

namespace BitPounce {
	
	static Ref<Audio> s_Audio;
	extern const std::filesystem::path g_AssetPath;
	
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
	{
	}
	
	void EditorLayer::OnAttach()
	{
		m_ActiveScene = CreateRef<Scene>();

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_PlayerTexture = Texture2D::Create("assets/textures/Player.png");
		m_Icon = Texture2D::Create("Resources/Icons/Icon.png");
		m_IconPlay = Texture2D::Create("Resources/Icons/Play.png");
		m_IconStop = Texture2D::Create("Resources/Icons/P.png");
	
	
		s_Audio =Audio::Create("assets/file_example_WAV_10MG.wav");
		s_Audio->Play();
	
		m_SceneHierarchyPanel = m_Panels.AddSystem<SceneHierarchyPanel>(m_ActiveScene);
		m_ContentBrowserPanel = m_Panels.AddSystem<ContentBrowserPanel>();
		m_Panels.Start();
	
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = Application::Get().GetWindow().GetWidth();
		fbSpec.Height = Application::Get().GetWindow().GetHeight();
		m_RendorSize = glm::vec2(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		m_Framebuffer = Framebuffer::Create(fbSpec);

		OnNewScene(m_ActiveScene);

	}
	
	void EditorLayer::OnDetach()
	{
		m_Panels.Stop();
	}
	
	void Dockspace(std::function<void()> callback)
	{
			// TL;DR; this demo is more complicated than what most users you would normally use.
		// If we remove all options we are showcasing, this demo would become a simple call to ImGui::DockSpaceOverViewport() !!
		// In this specific demo, we are not using DockSpaceOverViewport() because:
	
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}
	
		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
	
		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	
		bool is1x1is1 = 1 * 1 == 1;
		ImGui::Begin("DockSpace Demo", &is1x1is1, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();
	
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);
	
		// Submit the DockSpace
		// REMINDER: THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace()!
		// MOST REGULAR APPLICATIONS WILL SIMPLY WANT TO CALL DockSpaceOverViewport(). READ COMMENTS ABOVE.
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		else
		{
			BP_CORE_ASSERT(0,"NO DOCKING FOR EDITOR SOMEONE DISABLED IT WHYYYYYY!");
		}
		style.WindowMinSize.x = minWinSizeX;
	
		// Show demo options and help
		callback();
		
	
		ImGui::End();
	}
	
	void EditorLayer::OnUpdate(Timestep& ts)
	{
		m_time += ts;
	
		if(m_time >= 10)
		{
			s_Audio.reset();
		}
	
	
		// Update
		m_Panels.OnUpdate(ts);
		m_CameraController.OnUpdate(ts);
		m_EditorCamera.OnUpdate(ts);
	
		m_Framebuffer->Bind();
		// Render
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		m_Framebuffer->ClearAttachment(1, -1);
	
		m_Panels.OnDraw(ts);
	
		Renderer::BeginScene(m_CameraController.GetCamera());
	
		
		// Update scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				if (m_ViewportFocused)
					m_CameraController.OnUpdate(ts);

				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(ts);
				break;
			}
		}

		auto[mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			if (pixelData > m_ActiveScene->GetRegistry(*(ECSSystem*)0).view<entt::entity>().size())
			{
				// The gpu is a loves lieing to me ):
				m_HoveredEntity =  Entity();
			}
			else if(pixelData != -1)
			{
				m_HoveredEntity = Entity((entt::entity)pixelData, m_ActiveScene.get());
			}
			else
			{
				m_HoveredEntity =  Entity();
			}
			
		}
		
		
		Renderer::EndScene();
		m_Framebuffer->Unbind();
	}
	
	void EditorLayer::OnImGuiRender()
	{
		Dockspace([this]() { this->OnDockSpace(); });
	
	
	
	}
	
	void EditorLayer::OnDockSpace()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{

				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit")) Application::Get().Close(0);
					ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		UI_Toolbar();

		m_Panels.OnImGuiDraw();
	
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

		std::string name = "None";
		if (m_HoveredEntity)
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("Hovered Entity: %s", name.c_str());
		ImGui::End();
	
		ImGui::Begin("Render Data");
		Renderer2D::Renderer2DData renderer2d = Renderer2D::Get();
	
		ImGui::Text((std::string("Render Calls: ") + std::to_string(renderer2d.RenderCalls)).c_str());
		ImGui::Text((std::string("Tries: ") + std::to_string(renderer2d.Tries)).c_str());
		ImGui::Text((std::string("Quads: ") + std::to_string(renderer2d.Quads)).c_str());
		ImGui::Text((std::string("Indices: ") + std::to_string(renderer2d.Indices)).c_str());
		ImGui::Text((std::string("Vertices: ") + std::to_string(renderer2d.Vertices)).c_str());
	
	
		ImGui::End();
	
		ImGui::Begin("Systems");
	
		for(auto sys : m_ActiveScene->GetSysManager().Get())
		{
			ImGui::Text(sys->GetName().c_str());
		}
	
		ImGui::End();
	
		ImGui::Begin("Render");
		auto viewportOffset = ImGui::GetCursorPos(); // Includes tab bar
		if(ImGui::IsMouseDown(0) && m_HoveredEntity && m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_SceneHierarchyPanel->SetSelectedEntity(m_HoveredEntity);
		}

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);
	
		ImVec2 rendorPanelSize = ImGui::GetContentRegionAvail();
		glm::vec2 panelSize = { rendorPanelSize.x, rendorPanelSize.y };
		if (((m_RendorSize.x != panelSize.x) || (m_RendorSize.y != panelSize.y)) && ((m_RendorSize.x >= 1) || (m_RendorSize.x >= 1)))
		{
			m_Framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			m_RendorSize = {panelSize.x, panelSize.y};
			m_EditorCamera.SetViewportSize(panelSize.x, panelSize.y);
			m_CameraController.OnResize(panelSize.x, panelSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
		}
		
	
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, rendorPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(std::filesystem::path(g_AssetPath) / path);
			}
			ImGui::EndDragDropTarget();
		}

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + m_RendorSize.x, minBound.y + m_RendorSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
		auto cameraEntity = m_ActiveScene->GetActiveCamera();
		if (selectedEntity && m_GizmoType != -1 && cameraEntity.first)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			// Camera
			/*const auto& camera = cameraEntity.first->Camera;
			const glm::mat4& cameraProjection = camera.GetProjection();
			glm::mat4 cameraView = glm::inverse(cameraEntity.second->GetTransform());*/
			glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			if(m_SceneState == SceneState::Play)
			{
				const auto& camera = cameraEntity.first->Camera;
				cameraProjection = camera.GetProjection();
				cameraView = glm::inverse(cameraEntity.second->GetTransform());
			}

			

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftCtrl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::mat4 parentTransform = glm::mat4(1.0f);

				if (tc.Parent)
        			parentTransform = tc.Parent.GetComponent<TransformComponent>().GetTransform();

				glm::mat4 localTransform = glm::inverse(parentTransform) * transform;

				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(localTransform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}
	
		ImGui::End();
	}
	
	void EditorLayer::OnEvent(Event& e)
	{
		m_Panels.OnEvent(e);
		m_CameraController.OnEvent(e);
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BP_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}
	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(Key::LeftCtrl) || Input::IsKeyPressed(Key::RightCtrl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode())
		{
			case (int)Key::N:
			{
				if (control)
				{
					NewScene();
				}
					

				return false;
			}
			case (int)Key::O:
			{
				if (control)
				{
					OpenScene();
				}
					

				return false;
			}
			case (int)Key::S:
			{
				if (control && shift)
				{
					SaveSceneAs();
				}
					

				return false;
			}

			// Gizmos
			case (int)Key::Q:
				m_GizmoType = -1;
				return false;
			case (int)Key::W:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				return false;
			case (int)Key::E:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				return false;
			case (int)Key::R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				return false;
		}

		return false;
	}
	void EditorLayer::OnNewScene(Ref<Scene> scene)
	{
		scene->AddSystem<Renderer2DSystem>();
		scene->AddSystem<CameraSystem>();
		scene->AddSystem<Physics2DSystem>();
		scene->AddedAllSys();
	}
	void EditorLayer::NewScene()
	{
		// HACK: TODO FIX
		m_ActiveScene->RemoveAll();
		m_ActiveScene.reset();
		m_ActiveScene = CreateRef<Scene>("rttrgyuyuiyujk");
		OnNewScene(m_ActiveScene);
		
		m_ActiveScene->OnViewportResize((uint32_t)m_RendorSize.x, (uint32_t)m_RendorSize.y);
		m_SceneHierarchyPanel->SetContext(m_ActiveScene);
		m_EditorScene = m_ActiveScene;
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		NewScene();

		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize(path.string());
	}

	void EditorLayer::OpenScene()
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
		std::optional<std::string> filepath = FileDialogs::OpenFile("BitPounce Scene (*.bitPounce)\0*.bitPounce\0");
		if (filepath)
		{
			NewScene();

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(*filepath);
		}
	}
	void EditorLayer::SaveSceneAs()
	{
		std::filesystem::path filepath = FileDialogs::SaveFile("BitPounce Scene (*.bitPounce)\0*.bitPounce\0");
		if(!filepath.has_extension())
		{
			filepath = filepath.string() + ".bitPounce";
		}

		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath.string());
		}
	}

	void EditorLayer::UI_Toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;
		Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton( "##ToolbarButton", (ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
		{
			if (m_SceneState == SceneState::Edit)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneStop();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();
		m_SceneHierarchyPanel->SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = m_EditorScene;

		m_SceneHierarchyPanel->SetContext(m_ActiveScene);

	}
}