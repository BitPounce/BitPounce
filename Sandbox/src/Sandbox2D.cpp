#include "Sandbox2D.h"
#include "imgui.h"
#include "BitPounce.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

static BitPounce::Ref<BitPounce::Audio> s_Audio;

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = BitPounce::Texture2D::Create("assets/textures/Checkerboard.png");
	m_PlayerTexture = BitPounce::Texture2D::Create("assets/textures/Player.png");

	
	s_Audio =BitPounce::Audio::Create("assets/file_example_WAV_10MG.wav");
	s_Audio->Play();
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(BitPounce::Timestep& ts)
{
	m_time += ts;

	if(m_time >= 10)
	{
		s_Audio.reset();
	}


	// Update
	m_CameraController.OnUpdate(ts);

	// Render
	BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	BitPounce::RenderCommand::Clear();

	BitPounce::Renderer::BeginScene(m_CameraController.GetCamera());

	BitPounce::Renderer2D::BeginScene(m_CameraController.GetCamera());
	BitPounce::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_SquareColor);
	BitPounce::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });

	BitPounce::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10);

	BitPounce::Renderer2D::DrawRotatedQuad({ 1.5f, -1.5f }, { 0.9f, 0.75f }, glm::radians(m_time * 40), m_PlayerTexture, 1, m_SquareColor);
	BitPounce::Renderer2D::DrawRotatedQuad({ 1.5f, -2.5f }, { 0.9f, 0.75f }, glm::radians(m_time * 40), m_PlayerTexture);

	for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				BitPounce::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}

	#if !BP_RENDERER2D_USE_BATCH_RENDERING

	

	
	#endif

	BitPounce::Renderer2D::EndScene();

	BitPounce::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
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
}

void Sandbox2D::OnEvent(BitPounce::Event& e)
{
	m_CameraController.OnEvent(e);
}