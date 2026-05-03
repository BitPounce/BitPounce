#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"
#include "Example.h"
#include "Sandbox2D.h"
#include "ECSTest.h"

class Sandbox : public BitPounce::Application
{
public:

	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		//PushLayer(new Sandbox2D());
		PushLayer(new ECSTest());
	}

	~Sandbox()

	{
	}

private:

};

BitPounce::Application* CreateApp()
{
	return new Sandbox();
}