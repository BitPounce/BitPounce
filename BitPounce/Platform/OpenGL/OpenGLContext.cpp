#include <bp_pch.h>
#include "OpenGLContext.h"
#include "gl.h"
#include "BitPounce/Core/Base.h"
#include <GLFW/glfw3.h>

namespace BitPounce
{
    OpenGLContext::OpenGLContext(GLFWwindow* window)
        : m_WindowHandle(window)
    {
        BP_CORE_ASSERT(window, "Window handle is null!")
    }

    void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		

        #ifdef BP_PLATFORM_WEB
        int version = gladLoadGLSC2((GLADloadfunc)glfwGetProcAddress);
        #else
        int version = gladLoadGL(glfwGetProcAddress);
        #endif
		BP_CORE_ASSERT(version, "Failed to initialize Glad!");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}