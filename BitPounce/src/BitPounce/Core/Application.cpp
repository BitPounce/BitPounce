#include "Application.h"
#include "Logger.h"

namespace BitPounce
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	
	int Application::Run()
	{
		while (true)
		{
			Update();
		}

		return 0;
	}

	void Application::Update()
	{
		BP_CORE_INFO("Update called");
	}
}