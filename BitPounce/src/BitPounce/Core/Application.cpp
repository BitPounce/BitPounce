#include "bp_pch.h"

#include "Application.h"
#include "Logger.h"
#include <BitPounce/Events/ApplicationEvent.h>

namespace BitPounce
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
	}

	Application::~Application()
	{
	}

	
	int Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			BP_CORE_INFO("{}", e);
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			BP_CORE_INFO("{}", e);
		}

#ifdef BP_PLATFORM_WEB
        emscripten_set_main_loop([]() {
            s_Instance->Update();
        }, 0, true);
#else
        while (true)
        {
            Update();
        }
#endif

		return 0;
	}

	void Application::Update()
	{
		
	}
}