#include "bp_pch.h"

#include "Platform/OpenGL/gl.h"
#include "Application.h"
#include "Logger.h"
#include <BitPounce/Events/ApplicationEvent.h>

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

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
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

		for (Layer* layer : m_LayerStack)
			layer->OnUpdate();

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

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}

		Update();
		m_IsPoolingEvents = false;
	}
}