#include "bp_pch.h"

#include "Application.h"
#include "Logger.h"
#include <BitPounce/Events/ApplicationEvent.h>

#include <GLFW/glfw3.h>

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

namespace BitPounce
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		m_Window = Window::Create();
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::Close(int errorCode)
	{
		m_ErrorCode = errorCode;
		m_IsRunning = false;
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
        while (m_IsRunning)
        {
            Update();
        }
#endif

		return m_ErrorCode;
	}

	void Application::Update()
	{
		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		m_Window->OnUpdate(m_IsPoolingEvents);
	}
	void Application::OnEvent(Event& event)
	{
		m_IsPoolingEvents = true;
		EventDispatcher dispatcher = EventDispatcher(event);

		dispatcher.Dispatch<WindowCloseEvent>([&](WindowCloseEvent& _)
		{
			this->Close(0);
			return false;
		});

		BP_CORE_INFO("{}", event)

		Update();
		m_IsPoolingEvents = false;
	}
}