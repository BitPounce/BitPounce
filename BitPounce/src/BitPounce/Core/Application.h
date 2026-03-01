#pragma once

#include "Base.h"

#include "BitPounce/Core/LayerStack.h"
#include "BitPounce/Events/Event.h"
#include "Window.h"
#include "BitPounce/ImGui/ImGuiLayer.h"
#include "BitPounce/Core/Timestep.h"
#include "BitPounce/Events/ApplicationEvent.h"
namespace BitPounce
{
	struct ApplicationProps
	{
		std::string Title;
		std::string IconPath;
		uint32_t Width;
		uint32_t Height;

		ApplicationProps(const std::string& title = "BitPounce",
			uint32_t width = 1600,
			uint32_t height = 900, const std::string& iconPath = "NULL")
			: Title(title), Width(width), Height(height), IconPath(iconPath)
		{
		}
	};

	class Application
	{
	public:
		Application(const ApplicationProps& props = ApplicationProps());
		virtual ~Application();
		void Close(int errorCode);

		
		int Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() 
		{
			return *s_Instance;
		}

		Window& GetWindow()
		{
			return *m_Window;
		}

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	private:
		float m_LastFrameTime = 0.0f;
		void Update();
		ImGuiLayer* m_ImGuiLayer;
		static Application* s_Instance;
		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		int m_ErrorCode = -1;
		bool m_IsRunning = true;
		bool m_IsPoolingEvents = false;
		void OnEvent(Event& event);
		bool OnWindowResize(WindowResizeEvent& e);
		
	};

	
}
extern BitPounce::Application* CreateApp();