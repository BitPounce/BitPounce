#include <bp_pch.h>
#include "Window.h"

#ifdef BP_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

#ifdef BP_PLATFORM_WEB
#include "Platform/Web/WebWindow.h"
#endif


namespace BitPounce
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef BP_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#elifdef BP_PLATFORM_WEB
		return CreateScope<WebWindow>(props);
#else
		return nullptr;
#endif
	}
}