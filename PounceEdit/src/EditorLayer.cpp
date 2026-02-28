#include "EditorLayer.h"
#include "imgui.h"
#include "BitPounce.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class TestSystem : public BitPounce::System
{
public:
    TestSystem()
    {
        m_name = std::string("Test System");
    }

    virtual void Start() override
    {
        BP_CORE_INFO("STARTING TEST SYS");
    }
    virtual void Stop() override
    {
        BP_CORE_INFO("STOPING TEST SYS");
    }
};

static BitPounce::Ref<BitPounce::Audio> s_Audio;

EditorLayer::EditorLayer()
	: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
{
}

void EditorLayer::OnAttach()
{
	m_CheckerboardTexture = BitPounce::Texture2D::Create("assets/textures/Checkerboard.png");
	m_PlayerTexture = BitPounce::Texture2D::Create("assets/textures/Player.png");
    m_Icon = BitPounce::Texture2D::Create("assets/textures/Icon.png");

	
	s_Audio =BitPounce::Audio::Create("assets/file_example_WAV_10MG.wav");
	s_Audio->Play();

    m_SysManager.AddSystem<TestSystem>();
    m_SysManager.Start();

    BitPounce::FramebufferSpecification fbSpec;
	fbSpec.Width = 1600;
	fbSpec.Height = 900;
	m_Framebuffer = BitPounce::Framebuffer::Create(fbSpec);
}

void EditorLayer::OnDetach()
{
    m_SysManager.Stop();
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
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        BP_CORE_ASSERT(0,"NO DOCKING FOR EDITOR SOMEONE DISBALE IT WHYYYYYY!");
    }

    // Show demo options and help
    callback();
    

    ImGui::End();
}

void EditorLayer::OnUpdate(BitPounce::Timestep& ts)
{
	m_time += ts;

	if(m_time >= 10)
	{
		s_Audio.reset();
	}


	// Update
    m_SysManager.OnUpdate(ts);
	m_CameraController.OnUpdate(ts);

    m_Framebuffer->Bind();
	// Render
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();

    m_SysManager.OnDraw(ts);

	BitPounce::Renderer::BeginScene(m_CameraController.GetCamera());

	BitPounce::Renderer2D::BeginScene(m_CameraController.GetCamera());
	BitPounce::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_SquareColor);
	BitPounce::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });

	BitPounce::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_CheckerboardTexture, 20);

	BitPounce::Renderer2D::DrawRotatedQuad({ 1.5f, -1.5f }, { 0.9f, 0.75f }, glm::radians(m_time * 40), m_PlayerTexture, 1, m_SquareColor);
	BitPounce::Renderer2D::DrawRotatedQuad({ 1.5f, -2.5f }, { 0.9f, 0.75f }, glm::radians(m_time * 40), m_Icon);

	for (float y = -10.0f; y < 10.0f; y += 0.2f)
		{
			for (float x = -10.0f; x < 10.0f; x += 0.2f)
			{
				glm::vec4 color = { (x + 10.0f) / 20.0f, 0.4f, (y + 10.0f) / 20.0f, 0.7f };
				BitPounce::Renderer2D::DrawQuad({ x, y }, { 0.2f, 0.2f }, color);
			}
		}

	#if !BP_RENDERER2D_USE_BATCH_RENDERING

	

	
	#endif

	BitPounce::Renderer2D::EndScene();

	BitPounce::Renderer::EndScene();
    m_Framebuffer->Unbind();
}

void EditorLayer::OnImGuiRender()
{
	Dockspace([this]() { this->OnDockSpace(); });
	


}

void EditorLayer::OnDockSpace()
{
    m_SysManager.OnImGuiDraw();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();

	ImGui::Begin("Render Data");
	BitPounce::Renderer2D::Renderer2DData renderer2d = BitPounce::Renderer2D::Get();

	ImGui::Text((std::string("Render Calls: ") + std::to_string(renderer2d.RenderCalls)).c_str());
	ImGui::Text((std::string("Tries: ") + std::to_string(renderer2d.Tries)).c_str());
	ImGui::Text((std::string("Quads: ") + std::to_string(renderer2d.Quads)).c_str());
	ImGui::Text((std::string("Indices: ") + std::to_string(renderer2d.Indices)).c_str());
	ImGui::Text((std::string("Vertices: ") + std::to_string(renderer2d.Vertices)).c_str());
	

	ImGui::End();

    ImGui::Begin("Systems");

    for(auto sys : m_SysManager.Get())
    {
        ImGui::Text(sys->GetName().c_str());
    }

    ImGui::End();

    ImGui::Begin("Render");

    uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)textureID, ImVec2{ 1600, 900 });

    ImGui::End();
}

void EditorLayer::OnEvent(BitPounce::Event& e)
{
    m_SysManager.OnEvent(e);
	m_CameraController.OnEvent(e);
}