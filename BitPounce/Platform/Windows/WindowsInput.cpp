#include "bp_pch.h"
#include "WindowsInput.h"
#include <GLFW/glfw3.h>
#include "BitPounce/Core/Application.h"

namespace BitPounce
{
	Input* Input::s_Instance = new WindowsInput;

	int CustomKeyToGLFW(Key key)
{
    switch (key)
    {
#define BP_KEY(name, glfw) case Key::name: return glfw;
#include "Platform/GLFWCommon/KeyGLFW.def"
#undef BP_KEY
        default: return GLFW_KEY_UNKNOWN;
    }
}


	Key GLFWKeyToCustom(int glfwKey)
{
    switch (glfwKey)
    {
#define BP_KEY(name, glfw) case glfw: return Key::name;
#include "Platform/GLFWCommon/KeyGLFW.def"
#undef BP_KEY
        default: return Key::Esc; // or Key::Unknown if you add one
    }
}


	bool WindowsInput::IsKeyPressedImpl(Key key)
	{
		int glfwKey = CustomKeyToGLFW(key);
		return glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), glfwKey);
	}

    Key PlatformKeyToKey(uint32_t key)
    {
        return GLFWKeyToCustom(key);
    }
    uint32_t KeyToPlatformKey(Key key)
    {
        return CustomKeyToGLFW(key);
    }

    bool WindowsInput::IsMouseButtonPressedImpl(int button)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
    }

    glm::vec2 WindowsInput::GetMousePositionImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
    }

    float WindowsInput::GetMouseXImpl()
    {
        auto pos = GetMousePositionImpl();
		return pos.x;
    }

    float WindowsInput::GetMouseYImpl()
    {
        auto pos = GetMousePositionImpl();
		return pos.y;
    }

}