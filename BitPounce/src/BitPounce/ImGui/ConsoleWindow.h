#pragma once
#include <BitPounce/Core/Base.h>
#include <BitPounce/Core/Console.h>

namespace BitPounce
{
	class ImGuiConsoleWindow
	{
	public:
		ImGuiConsoleWindow();
		~ImGuiConsoleWindow();
		void OnImGuiDraw();
	private:
	};
}