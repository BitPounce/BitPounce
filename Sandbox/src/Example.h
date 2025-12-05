#pragma once
#include <BitPounce.h>


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
		BP_TRACE("{0}", event);
	}
};