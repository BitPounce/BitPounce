#include <BitPounce.h>
#include "ECSTest.h"

ECSTest::ECSTest() : m_Camera(-5, 5, -5, 5)
{
	
	
}

void ECSTest::OnAttach() 
{
	m_Project = BitPounce::Project::Load("assets/SandboxProject/SandboxProject.bpproj", false);
	BitPounce::AssetMap assetMap = m_Project->GetRuntimeAssetManager()->GetAssetMap();
	m_Project->GetSceneManager().AddAssetMap(assetMap);
	
}

void ECSTest::OnDetach() 
{
	
}

void ECSTest::OnUpdate(BitPounce::Timestep &ts)
{
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();

	m_Project->GetSceneManager().GetScene()->OnUpdateRuntime(ts);
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
	m_Project->GetSceneManager().GetScene()->OnImguiDraw(ts);
}

void ECSTest::OnEvent(BitPounce::Event &e)
{
	BitPounce::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<BitPounce::AssetPreLoadedEvent>(BP_BIND_EVENT_FN(ECSTest::OnAssetPreloaded));
	dispatcher.Dispatch<BitPounce::WindowResizeEvent>(BP_BIND_EVENT_FN(ECSTest::OnWindowResize));

	if(m_Project && m_Project->GetSceneManager().GetScene())
		m_Project->GetSceneManager().GetScene()->OnEvent(e);
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
	m_Project->GetSceneManager().GetScene()->OnViewportResize(e.GetWidth(), e.GetHeight());
    return false;
}
