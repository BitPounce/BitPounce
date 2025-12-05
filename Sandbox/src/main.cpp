#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"
#include "Example.h"

class Sandbox : public BitPounce::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer());
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