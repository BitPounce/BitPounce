#include "Application.h"
#include "Logger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace BitPounce
{
	Application* s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
	}

	Application::~Application()
	{
	}

	
	int Application::Run()
	{
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