#include "bp_pch.h"
#include "LinuxInput.h"
#include <GLFW/glfw3.h>
#include "BitPounce/Core/Application.h"

namespace BitPounce
{
    Input* Input::s_Instance = new LinuxInput;

    bool LinuxInput::IsKeyPressedImpl(KeyCode key)
    {
        return glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), key);
    }

}