#include "bp_pch.h"
#include "LinuxWindow.h"
#include "Platform/OpenGL/gl.h"
#include "BitPounce/Core/KeyCode.h"
#include "BitPounce/Events/KeyEvent.h"
#include "BitPounce/Events/ApplicationEvent.h"
#include "BitPounce/Events/MouseEvent.h"
#include "BitPounce/Core/Logger.h"
#include <stb_image.h>
#include "stb_image_resize2.h"

#include "Platform/OpenGL/OpenGLContext.h"

static void SetWindowIconSafe(GLFWwindow* window, const std::string& path)
{
    int w, h, channels;

    unsigned char* pixels =
        stbi_load(path.c_str(), &w, &h, &channels, 4);

    if (!pixels)
    {
        std::cerr << "Failed to load icon: " << path << std::endl;
        return;
    }

    const int MAX_ICON_SIZE = 256*2;

    unsigned char* finalPixels = pixels;
    int finalW = w;
    int finalH = h;

    // Auto-downscale if too large
    if (w > MAX_ICON_SIZE || h > MAX_ICON_SIZE)
    {
        float scale = std::min(
            (float)MAX_ICON_SIZE / w,
            (float)MAX_ICON_SIZE / h);

        finalW = (int)(w * scale);
        finalH = (int)(h * scale);

        finalPixels = new unsigned char[finalW * finalH * 4];

        stbir_resize_uint8_srgb(
            pixels, w, h, 0,
            finalPixels, finalW, finalH, 0,
            STBIR_RGBA);

        stbi_image_free(pixels);
    }

    GLFWimage image{};
    image.width  = finalW;
    image.height = finalH;
    image.pixels = finalPixels;

    glfwSetWindowIcon(window, 1, &image);

    if (finalPixels != pixels)
        delete[] finalPixels;
    else
        stbi_image_free(finalPixels);
}

namespace BitPounce {

	static bool s_GLFWInitialized = false;

	LinuxWindow::LinuxWindow(const WindowProps& props)
	{
		Init(props);
	}

	LinuxWindow::~LinuxWindow()
	{
		Shutdown();
	}

	void LinuxWindow::SetupGLFWCallback()
	{
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event((int)PlatformKeyToKey((uint32_t)key), 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event((int)PlatformKeyToKey((uint32_t)key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event((int)PlatformKeyToKey((uint32_t)key), 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void LinuxWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		BP_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			if (!success)
			{
				BP_CORE_CRITICAL("Could not initialize GLFW!");
				return;
			}

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		if (props.IconPath != "NULL")
    		SetWindowIconSafe(m_Window, props.IconPath);

		

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		SetupGLFWCallback();
	}

	void LinuxWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void LinuxWindow::OnUpdate(bool isPoolingEvents)
	{
		// HACK
		if (!isPoolingEvents)
		{
			glfwPollEvents();
		}
		
		m_Context->SwapBuffers();
	}

	void LinuxWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool LinuxWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void* LinuxWindow::GetNativeWindow() const
	{
		return m_Window;
	}

}