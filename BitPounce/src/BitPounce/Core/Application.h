#pragma once

#include "Base.h"

#include "BitPounce/Core/LayerStack.h"
#include "BitPounce/Events/Event.h"
#include "Window.h"
#include "BitPounce/ImGui/ImGuiLayer.h"
#include "BitPounce/Renderer/Shader.h"
#include "BitPounce/Renderer/Buffer.h"
#include "BitPounce/Renderer/VertexArray.h"

#include "BitPounce/Renderer/OrthographicCamera.h"

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
		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;
		OrthographicCamera m_Camera;

		std::shared_ptr<Shader> m_BlueShader;
		std::shared_ptr<VertexArray> m_SquareVA;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		void Update();
		ImGuiLayer* m_ImGuiLayer;
		static Application* s_Instance;
		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		int m_ErrorCode = -1;
		bool m_IsRunning = true;
		bool m_IsPoolingEvents = false;
		void OnEvent(Event& event);
		
	};

	
}
extern BitPounce::Application* CreateApp();