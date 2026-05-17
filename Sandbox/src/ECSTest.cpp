#include <BitPounce.h>
#include "ECSTest.h"
#include "GameInit.h"
#include "Shop.h"

//static BitPounce::Ref<BitPounce::Audio> s_Audio;

ECSTest::ECSTest() : m_Camera(-5, 5, -5, 5)
{

	Item item01 = {};
	item01.name = "2 guned";
	item01.fireRate = 9;
	item01.ID = 1;
	item01.Money = 100;
	item01.radius = 1.8f;
	item01.texHandle = 5260800240978246632;


	Item item03 = {};
	item03.name = "its lecking";
	item03.fireRate = 9;
	item03.ID = 2;
	item03.Money = 500;
	item03.radius = 2.5f;
	item03.texHandle = 2922514105519681440;

	Item item04 = {};
	item04.name = "The gun from Grab & Cash, if you know, you know";
	item04.fireRate = 10;
	item04.ID = 3;
	item04.Money = 1000;
	item04.radius = 3.f;
	item04.texHandle = 8357955715329051680;
	Shop_Load({item01, item03, item04});
	
}

struct OKLCH {
	double L;
	double C;
	double H;
};


OKLCH random_oklch(std::mt19937& gen)
{
	

	// Good practical ranges
	std::uniform_real_distribution<> lightness(0.6, 0.85);
	std::uniform_real_distribution<> chroma(0.08, 0.5);
	std::uniform_real_distribution<> hue(0.0, 360.0);

	return {
		lightness(gen),
		chroma(gen),
		hue(gen)
	};
}

struct RGBf {
	float r, g, b; // 0.0 - 1.0
};

RGBf oklch_to_rgbf(float L, float C, float H_deg)
{
	float h = H_deg * (float)IM_PI / 180.0f;

	// OKLCH -> OKLab
	float a = C * std::cos(h);
	float b = C * std::sin(h);

	// OKLab -> LMS
	float l_ = L + 0.3963377774f * a + 0.2158037573f * b;
	float m_ = L - 0.1055613458f * a - 0.0638541728f * b;
	float s_ = L - 0.0894841775f * a - 1.2914855480f * b;

	float l = l_ * l_ * l_;
	float m = m_ * m_ * m_;
	float s = s_ * s_ * s_;

	// LMS -> linear sRGB
	float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
	float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
	float bl = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

	// gamma correction (linear -> sRGB)
	auto gamma = [](float x) -> float
	{
		x = std::clamp(x, 0.0f, 1.5f);   // key fix

		if (x <= 0.0031308f)
			return 12.92f * x;

		return 1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f;
	};

	RGBf out;
	out.r = gamma(r);
	out.g = gamma(g);
	out.b = gamma(bl);

	return out;
}

void ECSTest::OnAttach() 
{
	m_Project = BitPounce::Project::Load("assets/MirrorDive/MirrorDive.bpproj", false);
	BitPounce::AssetMap assetMap = m_Project->GetRuntimeAssetManager()->GetAssetMap();

	
	m_Project->GetSceneManager().AddAssetMap(assetMap);
	//s_Audio =BitPounce::Audio::Create("assets/file_example_WAV_10MG.wav");
	//s_Audio->Play();
	//BitPounce::Entity ent = m_Project->GetSceneManager().GetScene()->CreateEntity("Test");
	//auto&& tilemap = ent.AddComponent<BitPounce::TilemapComponent>();
//
	//tilemap.renderer2D_tiles.push_back({glm::mat4(1), 547497271197996637});
	//GameCallbacks callbacks = {};
	//callbacks.PlayerDied = [this]()
	//{
	//	m_PlayerHasLost = true;
	//};
	//callbacks.InAWindow = [this](uint32_t seed)
	//{
	//	m_IsInAWindow = true;
	//	this->seed = seed;
	//};
	//m_Scene = GameLoad(callbacks, seed);

	BitPounce::FramebufferSpecification fbSpec;
	fbSpec.Attachments = { BitPounce::FramebufferTextureFormat::RGBA8, BitPounce::FramebufferTextureFormat::Depth };
	fbSpec.Width = BitPounce::Application::Get().GetWindow().GetWidth();
	fbSpec.Height = BitPounce::Application::Get().GetWindow().GetHeight();
	float aspectRatio = (float)fbSpec.Width / (float)fbSpec.Height;
	m_Camera.SetProjection(-aspectRatio * 5, aspectRatio * 5, -5, 5);
	m_Framebuffer = BitPounce::Framebuffer::Create(fbSpec);
	m_Shader = BitPounce::Shader::Create("assets/shaders/Test.glsl");
	m_Shader->Bind();
	m_Shader->SetInt("u_Texture", 0);
	m_Shader->SetInt("KERNEL_RADIUS", KERNEL_RADIUS);
	m_MainAudio = BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/ghfgfjjyfhgj.wav", true);
	m_MainAudio->Play();

	std::ifstream file("game.json");

	if(file.is_open())
	{
		nlohmann::json json;
		file >> json;

		if(json.contains("HighScore"))
			m_HighScore = json["HighScore"];

		file.close();
	}
	file = std::ifstream("settings.json");
	if (file.is_open())
	{
		nlohmann::json json;
		file >> json;

		BitPounce::AudioDevice::SetWorldVolume(json["Volume"]);
		KERNEL_RADIUS = json["KernelRadius"];
		m_Fullscreen = json["Fullscreen"];

		int w = json["Resolution"]["width"];
		int h = json["Resolution"]["height"];
		int refresh = json["Resolution"]["refreshRate"];

#ifndef BP_PLATFORM_WEB
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int modeCount = 0;
		const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
		for (int i = 0; i < modeCount; i++)
		{
			if (modes[i].width == w &&
				modes[i].height == h &&
				modes[i].refreshRate == refresh)
			{
				m_SelectedMode = i;
				break;
			}
		}

		m_ApplyDisplaySettings = true;
#endif
		file.close();
	}


	//m_Scene = BitPounce::CreateRef<BitPounce::Scene>();
	//m_Scene->CreateEntity("Camr")

	m_Shader->SetFloat4("u_Colour", glm::vec4(1));
}

void ECSTest::OnDetach() 
{
	
}

void ECSTest::OnUpdate(BitPounce::Timestep &ts)
{
	if (m_ApplyDisplaySettings)
	{
		m_ApplyDisplaySettings = false;

#ifndef BP_PLATFORM_WEB
		GLFWwindow* window =
			static_cast<GLFWwindow*>(BitPounce::Application::Get().GetWindow().GetNativeWindow());

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		int modeCount = 0;
		const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);

		const GLFWvidmode& mode = modes[m_SelectedMode];

		if (m_Fullscreen)
		{
			glfwSetWindowMonitor(window, monitor, 0, 0,
				mode.width, mode.height,
				mode.refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(window, nullptr, 100, 100,
				mode.width, mode.height, 0);
		}

		m_Framebuffer->Resize(mode.width, mode.height);

		float aspectRatio = (float)mode.width / (float)mode.height;
		m_Camera.SetProjection(-aspectRatio * 5, aspectRatio * 5, -5, 5);
#endif
	}

	if(m_IsInAWindow)
	{
		GameCallbacks callbacks = {};
		callbacks.PlayerDied = [this]()
		{
			m_PlayerHasLost = true;
			m_Shader->SetFloat4("u_Colour", glm::vec4(1));
		};
		callbacks.InAWindow = [this](uint32_t seed, std::mt19937& rng)
		{
			m_WindowsPlayerHasJumped++;
			if (m_WindowsToWin == m_WindowsPlayerHasJumped)
			{
				m_PlayerHasWined = true;
				m_Shader->SetFloat4("u_Colour", glm::vec4(1));
			}
			m_IsInAWindow = true;
			this->seed = seed;
			OKLCH oklch = random_oklch(rng);
			RGBf rgb = oklch_to_rgbf(oklch.L, oklch.C, oklch.H);
			m_Shader->SetFloat4("u_Colour", {rgb.r, rgb.g, rgb.b, 1});
		};
		callbacks.OnKilledEnemy = [this]()
		{
			m_Score += 10;
			if(m_Score > m_HighScore)
			{
				m_HighScore = m_Score;
			}
			nlohmann::json json = nlohmann::json();
			json["Score"] = m_Score;
			json["HighScore"] = m_HighScore;
			std::ofstream file("game.json");
			if (file.is_open())
			{
				file << json.dump(1, '\t');
				file.close();
			}
		};
		m_Scene = GameLoad(callbacks, seed);
		m_IsInAWindow = false;
	}
	m_Framebuffer->Bind();
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();

	if(m_Scene)
		m_Scene->OnUpdate(ts);
	BitPounce::Renderer2D::BeginScene(m_Camera);
	if(m_PlayerHasLost)
	{
		BitPounce::Renderer2D::DrawString("LOSER!!", BitPounce::AssetManager::GetAsset<BitPounce::Font>(1441554370544208483), glm::mat4(1), {});	
	}
	if (m_PlayerHasWined)
	{
		BitPounce::Renderer2D::DrawString("WIN!!", BitPounce::AssetManager::GetAsset<BitPounce::Font>(1441554370544208483), glm::mat4(1), {});

	}
	BitPounce::Renderer2D::DrawQuad(glm::vec2(0, 0), glm::vec2(1 ), glm::vec4(1));
	BitPounce::Renderer2D::DrawQuad(glm::vec2(0, 1), glm::vec2(1), glm::vec4(.9f));
	BitPounce::Renderer2D::DrawQuad(glm::vec2(1, 0), glm::vec2(1), glm::vec4(.9f));
	BitPounce::Renderer2D::DrawQuad(glm::vec2(1, 1), glm::vec2(1), glm::vec4(.9f));
	BitPounce::Renderer2D::DrawQuad(glm::vec2(0, 0), glm::vec2(1), glm::vec4(1));
	BitPounce::Renderer2D::EndScene();
	m_Framebuffer->Unbind();
	
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();
	glDepthMask(GL_FALSE);
	m_Shader->Bind();
	m_Shader->SetInt("KERNEL_RADIUS", KERNEL_RADIUS);
	m_Shader->SetInt("u_Texture", 0);
	//m_Shader->SetFloat3("u_Time", glm::vec3(glm::sin(m_Timer.Elapsed()), m_Timer.Elapsed(), glm::cos(m_Timer.Elapsed())));
	m_Framebuffer->BindAsTexture();
	BitPounce::Renderer::DrawFullScreenQuad();
	glDepthMask(GL_TRUE);

	if(m_PlayerHasLost || m_PlayerHasWined)
	{
		m_Scene.reset((BitPounce::Scene*)nullptr);
	}
}

void ECSTest::OnImGuiRender(BitPounce::Timestep& ts) 
{/*
	ImGui::Begin("Render Data");
	BitPounce::Renderer2D::Renderer2DData renderer2d = BitPounce::Renderer2D::Get();

	ImGui::Text((std::string("Render Calls: ") + std::to_string(renderer2d.RenderCalls)).c_str());
	ImGui::Text((std::string("Tries: ") + std::to_string(renderer2d.Tries)).c_str());
	ImGui::Text((std::string("Quads: ") + std::to_string(renderer2d.Quads)).c_str());
	ImGui::Text((std::string("Indices: ") + std::to_string(renderer2d.Indices)).c_str());
	ImGui::Text((std::string("Vertices: ") + std::to_string(renderer2d.Vertices)).c_str());
	

	ImGui::End();*/
	if(m_Scene)
		m_Scene->OnImguiDraw(ts);
	else
	{
		DrawMainMenu(ts);
	}
	if(m_ConsloeOpen)
		m_Console.OnImGuiDraw();
	Shop_ImGuiDraw();
	//m_Project->GetSceneManager().GetScene()->OnImguiDraw(ts);
}

void ECSTest::OnEvent(BitPounce::Event &e)
{
	BitPounce::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<BitPounce::AssetPreLoadedEvent>(BP_BIND_EVENT_FN(ECSTest::OnAssetPreloaded));
	dispatcher.Dispatch<BitPounce::WindowResizeEvent>(BP_BIND_EVENT_FN(ECSTest::OnWindowResize));
	dispatcher.Dispatch<BitPounce::KeyPressedEvent>([this](BitPounce::KeyPressedEvent& key)
	{
		if(key.GetKeyCode() == (int)BP_KEY_ESC)
		{
			m_ConsloeOpen = !m_ConsloeOpen;
		}
		return false;
	});
	if(m_Scene)
		m_Scene->OnEvent(e);
}

void ECSTest::DrawMainMenu(BitPounce::Timestep &ts)
{
	if (!m_hasReadedInfo)
	{
		ImGui::Begin("Info");
		ImGui::Text("This \"game\" is about mirror dimensions!");
		ImGui::Text("WASD to move");
		ImGui::Text("Jump into a mirror to enter the mirror dimensions");
		if (ImGui::Button("Yea! yea! i know")) 
		{
			m_hasReadedInfo = true;
		}
		ImGui::End();
		return;
	}

	ImGui::Begin("Main Menu");
	if(ImGui::Button("Play"))
	{
		m_PlayerHasLost = false;
		m_IsInAWindow = true;
		m_Score = 0;
		std::mt19937 rng(seed);
		OKLCH oklch = random_oklch(rng);
		RGBf rgb = oklch_to_rgbf(oklch.L, oklch.C, oklch.H);
		m_Shader->SetFloat4("u_Colour", { rgb.r, rgb.g, rgb.b, 1 });
		m_PlayerHasWined = false;
		m_WindowsToWin = 10;
		m_WindowsPlayerHasJumped = 0;
		
	}

	if (ImGui::Button("Play Inf"))
	{
		m_PlayerHasLost = false;
		m_IsInAWindow = true;
		m_Score = 0;
		std::mt19937 rng(seed);
		OKLCH oklch = random_oklch(rng);
		RGBf rgb = oklch_to_rgbf(oklch.L, oklch.C, oklch.H);
		m_Shader->SetFloat4("u_Colour", { rgb.r, rgb.g, rgb.b, 1 });
		m_PlayerHasWined = false;
		m_WindowsToWin = -1;
		m_WindowsPlayerHasJumped = 0;

	}
	ImGui::Text((std::string("Score: ") + std::to_string(m_Score)).c_str());
	ImGui::Text((std::string("High Score: ") + std::to_string(m_HighScore)).c_str());

	if(ImGui::Button("Settings"))
	{
		m_IsSettingsWindowOpen = !m_IsSettingsWindowOpen;
	}

	if(m_IsSettingsWindowOpen)
		DrawSetingsMenu(ts);
	
#ifndef BP_PLATFORM_WEB
	if(ImGui::Button("Exit"))
	{
		BitPounce::Application::Get().Close(0);
		
	}
#endif
	ImGui::End();
}

void ECSTest::DrawSetingsMenu(BitPounce::Timestep &ts)
{
	#ifndef BP_PLATFORM_WEB
	GLFWwindow* window = static_cast<GLFWwindow*>(BitPounce::Application::Get().GetWindow().GetNativeWindow());
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	int modeCount = 0;
	const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
	#endif

	ImGui::Begin("Settings", &m_IsSettingsWindowOpen);
	#ifndef BP_PLATFORM_WEB

	ImGui::Checkbox("Fullscreen", &m_Fullscreen);
	std::string currentRes = std::to_string(modes[m_SelectedMode].width) + " x " + std::to_string(modes[m_SelectedMode].height) + " @" + std::to_string(modes[m_SelectedMode].refreshRate) + "hz";
	if(ImGui::BeginCombo("Resolution", currentRes.c_str()))
	{
		for(int i = 0; i < modeCount; i++)
		{
			std::string label = std::to_string(modes[i].width) + " x " + std::to_string(modes[i].height) + " @" + std::to_string(modes[i].refreshRate) + "hz";
			bool isSelected = (m_SelectedMode == i);
			ImGui::PushID((label + std::to_string(i)).c_str());
			if(ImGui::Selectable(label.c_str(), isSelected))
			{
				m_SelectedMode = i;
			}
			ImGui::PopID();
			if(isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if(ImGui::Button("Apply"))
	{
		m_ApplyDisplaySettings = true;
		
	}
	#endif

	{
		float temp = BitPounce::AudioDevice::GetWorldVolume() * 100.f;
		if(ImGui::SliderFloat("Volume", &temp, 0.0f, 1.5f * 100.f))
		{
			BitPounce::AudioDevice::SetWorldVolume(temp / 100.f);
		}
		ImGui::SliderInt("Kernel radius", &KERNEL_RADIUS, 1, 30);
	}
	if (ImGui::Button("Save"))
	{
		const GLFWvidmode& mode = modes[m_SelectedMode];

		nlohmann::json json;
		json["Volume"] = BitPounce::AudioDevice::GetWorldVolume();
		json["KernelRadius"] = KERNEL_RADIUS;

		json["Fullscreen"] = m_Fullscreen;
		json["Resolution"] = {{"width", mode.width}, {"height", mode.height},{"refreshRate", mode.refreshRate}};

		std::ofstream file("settings.json");
		file << json.dump(1, 9);
	}
	ImGui::End();
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
}

bool ECSTest::OnWindowResize(BitPounce::WindowResizeEvent &e)
{
	if(m_Scene)
		m_Scene->OnViewportResize(e.GetWidth(), e.GetHeight());

	m_Framebuffer->Resize(e.GetWidth(), e.GetHeight());
	float aspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
	m_Camera.SetProjection(-aspectRatio * 5, aspectRatio * 5, -5, 5);
	return false;
}
