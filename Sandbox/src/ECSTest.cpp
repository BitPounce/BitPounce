#include <BitPounce.h>
#include "ECSTest.h"
#include "GameInit.h"

static BitPounce::Ref<BitPounce::Audio> s_Audio;

ECSTest::ECSTest() : m_Camera(-5, 5, -5, 5)
{
	
	
}

void ECSTest::OnAttach() 
{
	m_Project = BitPounce::Project::Load("assets/SandboxProject/SandboxProject.bpproj", false);
	BitPounce::AssetMap assetMap = m_Project->GetRuntimeAssetManager()->GetAssetMap();

	
	m_Project->GetSceneManager().AddAssetMap(assetMap);
	s_Audio =BitPounce::Audio::Create("assets/file_example_WAV_10MG.wav");
	s_Audio->Play();
	//BitPounce::Entity ent = m_Project->GetSceneManager().GetScene()->CreateEntity("Test");
	//auto&& tilemap = ent.AddComponent<BitPounce::TilemapComponent>();
//
	//tilemap.renderer2D_tiles.push_back({glm::mat4(1), 547497271197996637});
	m_Scene = GameLoad();

	BitPounce::FramebufferSpecification fbSpec;
	fbSpec.Attachments = { BitPounce::FramebufferTextureFormat::RGBA8, BitPounce::FramebufferTextureFormat::Depth };
	fbSpec.Width = BitPounce::Application::Get().GetWindow().GetWidth();
	fbSpec.Height = BitPounce::Application::Get().GetWindow().GetHeight();
	m_Framebuffer = BitPounce::Framebuffer::Create(fbSpec);
	m_Shader = BitPounce::Shader::Create("assets/shaders/Test.glsl");
	m_Shader->Bind();
	m_Shader->SetInt("u_Texture", 0);
}

void ECSTest::OnDetach() 
{
	
}

void ECSTest::OnUpdate(BitPounce::Timestep &ts)
{
	m_Framebuffer->Bind();
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();

	m_Scene->OnUpdate(ts);
	m_Framebuffer->Unbind();
	
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();
	m_Shader->Bind();
	m_Shader->SetInt("u_Texture", 0);
	m_Framebuffer->BindAsTexture();
	BitPounce::Renderer::DrawFullScreenQuad();
}

void ECSTest::OnImGuiRender(BitPounce::Timestep& ts) 
{
	ImGui::Begin("Render Data");
	BitPounce::Renderer2D::Renderer2DData renderer2d = BitPounce::Renderer2D::Get();

	ImGui::Text((std::string("Render Calls: ") + std::to_string(renderer2d.RenderCalls)).c_str());
	ImGui::Text((std::string("Tries: ") + std::to_string(renderer2d.Tries)).c_str());
	ImGui::Text((std::string("Quads: ") + std::to_string(renderer2d.Quads)).c_str());
	ImGui::Text((std::string("Indices: ") + std::to_string(renderer2d.Indices)).c_str());
	ImGui::Text((std::string("Vertices: ") + std::to_string(renderer2d.Vertices)).c_str());
	

	ImGui::End();
	m_Scene->OnImguiDraw(ts);
	m_Console.OnImGuiDraw();
	//m_Project->GetSceneManager().GetScene()->OnImguiDraw(ts);
}

void ECSTest::OnEvent(BitPounce::Event &e)
{
	BitPounce::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<BitPounce::AssetPreLoadedEvent>(BP_BIND_EVENT_FN(ECSTest::OnAssetPreloaded));
	dispatcher.Dispatch<BitPounce::WindowResizeEvent>(BP_BIND_EVENT_FN(ECSTest::OnWindowResize));
	if(m_Scene)
		m_Scene->OnEvent(e);
}

bool ECSTest::OnAssetPreloaded(BitPounce::AssetPreLoadedEvent &e)
{
	if(e.GetMetadata().Type == BitPounce::AssetType::Scene)
		{
			return OnScenePreloaded(e);
		}
    return false;
}

bool ECSTest::OnScenePreloaded(BitPounce::AssetPreLoadedEvent &e)
{
	// Yes, this causes a memory leak. Too bad!
		BitPounce::SceneAssetMetadata* sceneAssetMetadata = new BitPounce::SceneAssetMetadata();
		sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::Renderer2DSystem>());
		sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::CameraSystem>());
		sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::Physics2DSystem>());
		sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::AngelScriptSystem>());

		e.GetMetadata().data = std::optional<void*>((void*)sceneAssetMetadata);
        return false;
    return false;
}

bool ECSTest::OnWindowResize(BitPounce::WindowResizeEvent &e)
{
	m_Scene->OnViewportResize(e.GetWidth(), e.GetHeight());
	m_Framebuffer->Resize(e.GetWidth(), e.GetHeight());
    return false;
}
