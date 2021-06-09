#include "GameBuff.h"
#include "imgui/imgui.h"
#include "Color.h"
#include "Debug.h"

void GameBuff::ImGuiDraw()
{
	ImGui::TextColored(Color::YELLOW, name.c_str());
	ImGui::DragInt("Address", &address, 1.f, 0, 0, "%#10x");
	ImGui::DragFloat("Start Time", &startTime);
	ImGui::DragFloat("End Time", &endTime);
	ImGui::DragInt("Count", &count);
	ImGui::DragFloat("Value", &value);
}

int BuffEntry::GetCount() const
{
	return (buffNodeEnd - buffNodeStart) / 0x8;
}
