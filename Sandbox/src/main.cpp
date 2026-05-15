#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"
#include "Example.h"
#include "Sandbox2D.h"
#include "ECSTest.h"

class Sandbox : public BitPounce::Application
{
public:

	Sandbox(const BitPounce::ApplicationProps& props = BitPounce::ApplicationProps()) : Application(props)
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
	BitPounce::ApplicationProps props{};
	props.Title = "Mirror Dive";
	props.IconPath = "assets/icon.png";
	return new Sandbox(props);
}