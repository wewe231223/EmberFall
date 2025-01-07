#include <corecrt_wtime.h>
#include <format>
#include <chrono>
#include <algorithm>
#include "pch.h"
#include "Console.h"
#include "../External/Include/ImGui/imgui.h"
#include "../External/Include/ImGui/imgui_color.h"
#include "../Config/Config.h"
constexpr ImVec4 INFO_COLOR{ F4_YELLOW_GREEN };
constexpr ImVec4 WARNING_COLOR{ F4_WHEAT };
constexpr ImVec4 ERROR_COLOR{ F4_RED };

ConsoleBase::ConsoleBase()
{
}

ConsoleBase::~ConsoleBase()
{

}

void ConsoleBase::Render()
{
	// 고정된 위치와 크기
	ImVec2 fixedPos = ImVec2(0, 0);   // 창의 고정 위치
	ImVec2 fixedSize = ImVec2(400, 300);  // 창의 고정 크기

	// 창의 위치와 크기를 고정
	ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
	ImGui::SetNextWindowSize(ImVec2{ Config::EDITOR_WINDOW_WIDTH<>,Config::EDITOR_WINDOW_HEIGHT<> / 3 });

	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(mBuffer.Size()));

	while (clipper.Step()) {
		for (auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
			auto& [msg, type] = mBuffer[i];

			switch (type) {
			case LogType::Info:
				ImGui::TextColored(INFO_COLOR, msg.first.c_str());
				ImGui::SameLine(0.f,10.f);
				ImGui::TextWrapped(msg.second.c_str());
				break;
			case LogType::Warning:
				ImGui::TextColored(WARNING_COLOR, msg.first.c_str());
				ImGui::SameLine(0.f, 10.f);
				ImGui::TextWrapped(msg.second.c_str());
				break;
			case LogType::Error:
				ImGui::TextColored(ERROR_COLOR, msg.first.c_str());
				ImGui::SameLine(0.f, 10.f);
				ImGui::TextWrapped(msg.second.c_str());
				break;
			default:
				break;
			}

		}
	}

	ImGui::SetScrollHereY(1.0f);
	ImGui::End();
}


ConsoleBase Console{};