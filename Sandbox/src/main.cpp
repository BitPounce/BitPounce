#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"
#include "Example.h"
#include "Sandbox2D.h"
#include "ECSTest.h"
#include "3DTest.h"

class Sandbox : public BitPounce::Application
{
public:

	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		//PushLayer(new Sandbox2D());
		BitPounce::UILayer* UILayer = new BitPounce::UILayer();
		//PushLayer(new ECSTest(UILayer));
		PushLayer(new Test3D(UILayer));
		//PushOverlay(UILayer);
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