#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"
#include "Example.h"
#include "EditorLayer.h"

class Editor : public BitPounce::Application
{
public:

	Editor(const BitPounce::ApplicationProps& props): Application(props)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new BitPounce::EditorLayer());
	}

	~Editor()

	{
	}

private:

};

BitPounce::Application* CreateApp()
{
	BitPounce::ApplicationProps props = BitPounce::ApplicationProps();
	props.IconPath = "assets/textures/Icon.png";
	props.Title = "PounceEdit";

	return new Editor(props);
}