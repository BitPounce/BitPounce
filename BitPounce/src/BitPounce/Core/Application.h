#pragma once

#include "Base.h"

#include "BitPounce/Core/LayerStack.h"
#include "BitPounce/Events/Event.h"
#include "Window.h"
#include "BitPounce/ImGui/ImGuiLayer.h"
#include "BitPounce/Renderer/Shader.h"

namespace BitPounce
{
	class Application
	{
	public:
		Application();
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
		unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
		void Update();
		ImGuiLayer* m_ImGuiLayer;
		static Application* s_Instance;
		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		int m_ErrorCode = -1;
		bool m_IsRunning = true;
		bool m_IsPoolingEvents = false;
		std::unique_ptr<Shader> m_Shader;
		void OnEvent(Event& event);
		
	};

	
}
extern BitPounce::Application* CreateApp();