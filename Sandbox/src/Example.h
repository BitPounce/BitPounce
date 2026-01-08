#pragma once
#include <BitPounce.h>
#include <imgui.h>

class ExampleLayer : public BitPounce::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		//BP_INFO("ExampleLayer::Update");
	}

	void OnEvent(BitPounce::Event& event) override
	{
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Debug");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0/(ImGui::GetIO().Framerate),(ImGui::GetIO().Framerate));
		ImGui::End();
	}
};