#include <BitPounce.h>
#include "BitPounce/Core/EntryPoint.h"


class Sandbox : public BitPounce::Application
{
public:

	Sandbox()
	{
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