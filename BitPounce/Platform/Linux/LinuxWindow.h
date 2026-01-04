#pragma once
#include "BitPounce/Core/Window.h"
#include <GLFW/glfw3.h>
#include "BitPounce/Renderer/GraphicsContext.h"

namespace BitPounce
{
	class LinuxWindow : public Window
	{
	public:
		LinuxWindow(const WindowProps& props);
		virtual ~LinuxWindow();

		void OnUpdate(bool isPoolingEvents = false) override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		

		virtual void* GetNativeWindow() const override;
	private:
		void SetupGLFWCallback();
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}