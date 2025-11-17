#include "Application.h"
#include "Logger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
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

#ifdef __EMSCRIPTEN__
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
		BP_CORE_INFO("Update called");
	}
}