#pragma once

#include "Base.h"
#include "BitPounce/Events/Event.h"
#include "Window.h"

namespace BitPounce
{
	class Application
	{
	public:
		Application();
		virtual ~Application();
		void Close(int errorCode);
		
		int Run();
	private:
		
		void Update();
		static Application* s_Instance;
		Scope<Window> m_Window;
		int m_ErrorCode = -1;
		bool m_IsRunning = true;
		bool m_IsPoolingEvents = false;
		void OnEvent(Event& event);
	};

	
}
extern BitPounce::Application* CreateApp();