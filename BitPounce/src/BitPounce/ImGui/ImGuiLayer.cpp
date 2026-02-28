#include "bp_pch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "BitPounce/Core/Application.h"
#include <GLFW/glfw3.h>
#include <BitPounce/Core/Logger.h>

namespace BitPounce {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") { }

ImGuiLayer::~ImGuiLayer() { }

void ImGuiLayer::OnAttach()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Config flags
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    #ifndef __EMSCRIPTEN__
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     
    #endif

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Application& app = Application::Get();
    GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
    if (!window)
    {
        BP_CORE_ERROR("ImGuiLayer: Failed to get GLFW window!");
        return;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    #ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
    #else
    ImGui_ImplOpenGL3_Init("#version 410");
    #endif
}

void ImGuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End()
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();

    io.DisplaySize = ImVec2(
        static_cast<float>(app.GetWindow().GetWidth()),
        static_cast<float>(app.GetWindow().GetHeight())
    );

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle multi-viewport rendering, whyyy
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* mainWindow =
            static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        GLFWwindow* backup_context = glfwGetCurrentContext();

        glfwMakeContextCurrent(mainWindow);

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_context);
    }
}

void ImGuiLayer::OnImGuiRender()
{
}

} // namespace BitPounce
