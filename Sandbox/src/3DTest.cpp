#include <BitPounce.h>
#include "3DTest.h"
static BitPounce::Ref<BitPounce::Audio> s_Audio;

// Vertices coordinates
BitPounce::MeshVertex vertices[] =
{ //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
	BitPounce::MeshVertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f)},
	BitPounce::MeshVertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f)},
	BitPounce::MeshVertex{glm::vec3( 1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f)},
	BitPounce::MeshVertex{glm::vec3( 1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f)}
};

// Indices for vertices order
uint32_t indices[] =
{
	0, 1, 2,
	0, 2, 3
};


Test3D::Test3D(BitPounce::UILayer* ui) : m_Camera(90, 1280.0f / 720.0f, 0.1f, 100.0f)
{

	m_UILayer = ui;
}

void Test3D::OnAttach() 
{
	m_Project = BitPounce::Project::Load("assets/SandboxProject/SandboxProject.bpproj", false);
	BitPounce::AssetMap assetMap = m_Project->GetRuntimeAssetManager()->GetAssetMap();
	
	m_Project->GetSceneManager().AddAssetMap(assetMap);
	for(auto&& asset : assetMap)
	{
		if(asset.second->GetType() == BitPounce::AssetType::Audio)
		{
			s_Audio = BitPounce::AssetManager::GetAsset<BitPounce::Audio>(asset.first);
			s_Audio->SetLooping(true);
		}
	}
	
	s_Audio->Play();
	//BitPounce::Entity ent = m_Project->GetSceneManager().GetScene()->CreateEntity("Test");
	//auto&& tilemap = ent.AddComponent<BitPounce::TilemapComponent>();

	//tilemap.renderer2D_tiles.push_back({glm::mat4(1), 547497271197996637});

	auto root = BitPounce::CreateRef<BitPounce::UIRoot>();
	root->SetPosition(glm::vec2(100.0f, 100.0f));

	// Add a red rectangle
	auto buttonContent = std::make_unique<BitPounce::UIImageElement>(BitPounce::AssetManager::GetAsset<BitPounce::Texture2D>(547497271197996637));
	BitPounce::UIButtonElement* button = new BitPounce::UIButtonElement(FBounds2(glm::vec2(50.0f, 150.0f), glm::vec2(150.0f, 50.0f)), std::move(buttonContent));
	button->SetOnClickCallback([]() { BP_INFO("Button clicked!"); });
	button->SetOnHoveredCallback([]() { BP_INFO("Button Hovered"); });
	root->AddElement(std::unique_ptr<BitPounce::UIElement>(button));

	m_UILayer->AddRoot(root);

	m_Shader = BitPounce::Shader::Create("assets/shaders/3D.glsl");
	m_Shader->Bind();
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
	m_Shader->SetInt("tex0", 0);
	m_Shader->SetFloat4("lightColor", lightColor);
	m_Shader->SetFloat3("lightPos", lightPos);
	//m_Tex = BitPounce::AssetManager::GetAsset<BitPounce::Texture2D>(6472988073815204772);

	std::pair<BitPounce::TextureType, BitPounce::Ref<BitPounce::Texture2D>> textures[]
	{
		{BitPounce::TextureType::Diffuse, BitPounce::AssetManager::GetAsset<BitPounce::Texture2D>(10233731665991405532)},
		{BitPounce::TextureType::Specular, BitPounce::AssetManager::GetAsset<BitPounce::Texture2D>(13123200920654724351)},
	};

	std::vector <BitPounce::MeshVertex> verts(vertices, vertices + sizeof(vertices) / sizeof(BitPounce::MeshVertex));
	std::vector <uint32_t> ind(indices, indices + sizeof(indices) / sizeof(uint32_t));
	std::vector <std::pair<BitPounce::TextureType, BitPounce::Ref<BitPounce::Texture2D>>> tex(textures, textures + sizeof(textures) / sizeof(std::pair<BitPounce::TextureType, BitPounce::Ref<BitPounce::Texture2D>>));
	m_Mesh = BitPounce::Mesh::Create(verts, ind, tex);

	
}

void Test3D::OnDetach() 
{
	
}

void Test3D::OnUpdate(BitPounce::Timestep &ts)
{
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();
	m_Camera.OnUpdate(ts);
	static float s_Timer = 0;
	s_Timer += ts;
	m_Shader->Bind();
	glm::mat4 model = BitPounce::Math::ComposeTransform({0,0,-10}, {glm::radians(s_Timer * 10.0f), glm::radians(s_Timer * 10.0f), glm::radians(s_Timer * 10.0f)}, {10,10,10});
	
	m_Shader->SetMat4("model", model);
	m_Shader->SetMat4("camMatrix", m_Camera.GetViewProjection());
	m_Shader->SetFloat("scale", 0.5f);
	glm::vec3 lightPos = glm::vec3(0.5f, sinf(s_Timer), 0.5f);
	m_Shader->SetFloat3("lightPos", lightPos);
	m_Shader->SetFloat3("camPos", m_Camera.GetPosition());
	BitPounce::RenderCommand::DrawIndexed(m_Mesh, m_Shader);
	//m_Project->GetSceneManager().GetScene()->OnUpdateRuntime(ts);
}

void Test3D::OnImGuiRender(BitPounce::Timestep& ts) 
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

void Test3D::OnEvent(BitPounce::Event &e)
{
	m_Camera.OnEvent(e);

	BitPounce::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<BitPounce::AssetPreLoadedEvent>(BP_BIND_EVENT_FN(Test3D::OnAssetPreloaded));
	dispatcher.Dispatch<BitPounce::WindowResizeEvent>(BP_BIND_EVENT_FN(Test3D::OnWindowResize));

	if(m_Project && m_Project->GetSceneManager().GetScene())
		m_Project->GetSceneManager().GetScene()->OnEvent(e);
}

bool Test3D::OnAssetPreloaded(BitPounce::AssetPreLoadedEvent &e)
{
	if(e.GetMetadata().Type == BitPounce::AssetType::Scene)
	{
		return OnScenePreloaded(e);
	}
	return false;
}

bool Test3D::OnScenePreloaded(BitPounce::AssetPreLoadedEvent &e)
{
	// Yes, this causes a memory leak. Too bad!
	BitPounce::SceneAssetMetadata* sceneAssetMetadata = new BitPounce::SceneAssetMetadata();
	sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::Renderer2DSystem>());
	sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::CameraSystem>());
	sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::Physics2DSystem>());
	sceneAssetMetadata->Systems.push_back(BitPounce::CreateRef<BitPounce::AngelScriptSystem>());

	e.GetMetadata().data = std::optional<void*>((void*)sceneAssetMetadata);
	return false;
}

bool Test3D::OnWindowResize(BitPounce::WindowResizeEvent &e)
{
	m_Project->GetSceneManager().GetScene()->OnViewportResize(e.GetWidth(), e.GetHeight());
	m_Camera.SetViewportSize(e.GetWidth(), e.GetHeight());
	return false;
}
