#pragma once
#include <string>

#include "BitPounce/Core/Base.h"
#include "BitPounce/Events/Event.h"

namespace BitPounce
{
	struct WindowProps
	{
		std::string Title;
		std::string IconPath;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "BitPounce",
			uint32_t width = 1600,
			uint32_t height = 900, const std::string& iconPath = "NULL")
			: Title(title), Width(width), Height(height), IconPath(iconPath)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate(bool isPoolingEvents = false) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}