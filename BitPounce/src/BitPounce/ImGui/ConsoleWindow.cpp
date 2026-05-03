#include <bp_pch.h>
#include "ConsoleWindow.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace BitPounce
{
	ImGuiConsoleWindow::ImGuiConsoleWindow()
	{
	}

	ImGuiConsoleWindow::~ImGuiConsoleWindow()
	{
	}
	void ImGuiConsoleWindow::OnImGuiDraw()
	{
		static int selectedItem = 0;
		static char CMDBuffer[256] = { '\0' };
		const bool is_multiline = false;
		ImGuiContext& g = * ImGui::GetCurrentContext();
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;
		const ImVec2 label_size = ImGui::CalcTextSize(CMDBuffer, NULL, true);
		float InputTextSize = (is_multiline ? g.FontSize * 8.0f : label_size.y) + style.FramePadding.y * 2.0f;

		const auto& logs = Console::GetLogs();

		std::vector<const char*> items;
		items.reserve(logs.size());

		for (const auto& log : logs)
		{
			items.push_back(log.c_str());
		}

		ImGui::Begin("Console");
		ImGui::PushItemWidth(-1);
		const float inputHeight = ImGui::GetFrameHeightWithSpacing();

		ImGui::BeginChild("LogRegion", ImVec2(0, -inputHeight), true);
			
		for (const auto& log : logs)
		{
			ImGui::TextUnformatted(log.c_str());
		}
		
		ImGui::EndChild();
		ImGui::Separator();

		if (ImGui::InputText("CMD", CMDBuffer, IM_ARRAYSIZE(CMDBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Console::Execute(CMDBuffer);
			memset(CMDBuffer, 0, sizeof(CMDBuffer));
		}
		ImGui::PopItemWidth();

		ImGui::End();
	}
}