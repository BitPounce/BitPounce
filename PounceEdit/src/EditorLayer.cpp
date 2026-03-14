#include "EditorLayer.h"
#include "imgui.h"
#include "BitPounce.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace BitPounce {
	
	static Ref<Audio> s_Audio;
	
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
	{
	}
	
	void EditorLayer::OnAttach()
	{
		m_ActiveScene = CreateRef<Scene>();

		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_PlayerTexture = Texture2D::Create("assets/textures/Player.png");
		m_Icon = Texture2D::Create("assets/textures/Icon.png");
	
	
		s_Audio =Audio::Create("assets/file_example_WAV_10MG.wav");
		s_Audio->Play();
	
		m_SceneHierarchyPanel = *m_Panels.AddSystem<SceneHierarchyPanel>(m_ActiveScene);
		m_Panels.Start();
	
		FramebufferSpecification fbSpec;
		fbSpec.Width = Application::Get().GetWindow().GetWidth();
		fbSpec.Height = Application::Get().GetWindow().GetHeight();
		m_RendorSize = glm::vec2(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		m_Framebuffer = Framebuffer::Create(fbSpec);

		
		m_ActiveScene->AddSystem<Renderer2DSystem>();
		m_ActiveScene->AddSystem<CameraSystem>();
		m_ActiveScene->AddedAllSys();
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
	
		m_Framebuffer->Bind();
		// Render
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
	
		m_Panels.OnDraw(ts);
	
		Renderer::BeginScene(m_CameraController.GetCamera());
	
		
		// Update scene
		m_ActiveScene->OnUpdate(ts);
		
		
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

				if (ImGui::MenuItem("Serialize"))
				{
					SceneSerializer serializer(m_ActiveScene);
					serializer.Serialize("assets/scenes/Example.bitPounce");
				}

				if (ImGui::MenuItem("Deserialize"))
				{
					SceneSerializer serializer(m_ActiveScene);
					serializer.Deserialize("assets/scenes/Example.bitPounce");
				}

				if (ImGui::MenuItem("Exit")) Application::Get().Close(0);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		m_Panels.OnImGuiDraw();
	
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
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
	
		for(auto sys : m_Panels.Get())
		{
			ImGui::Text(sys->GetName().c_str());
		}
	
		ImGui::End();
	
		ImGui::Begin("Render");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
	
		ImVec2 rendorPanelSize = ImGui::GetContentRegionAvail();
		glm::vec2 panelSize = { rendorPanelSize.x, rendorPanelSize.y };
		if (((m_RendorSize.x != panelSize.x) || (m_RendorSize.y != panelSize.y)) && ((m_RendorSize.x >= 1) || (m_RendorSize.x >= 1)))
		{
			m_Framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			m_RendorSize = {panelSize.x, panelSize.y};
		
			m_CameraController.OnResize(panelSize.x, panelSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
		}
		
	
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, rendorPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	
		ImGui::End();
	}
	
	void EditorLayer::OnEvent(Event& e)
	{
		m_Panels.OnEvent(e);
		m_CameraController.OnEvent(e);
	}
}