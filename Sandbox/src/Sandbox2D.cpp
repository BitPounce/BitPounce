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
	BitPounce::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture);

	BitPounce::Renderer2D::EndScene();

	BitPounce::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(BitPounce::Event& e)
{
	m_CameraController.OnEvent(e);
}