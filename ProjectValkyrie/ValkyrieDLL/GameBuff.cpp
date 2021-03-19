#include "GameBuff.h"
#include "imgui/imgui.h"
#include "Color.h"

void GameBuff::ReadFromBaseAddress(int addr)
{
	static float* GameTime = (float*)((int)GetModuleHandle(NULL) + Offsets::GameTime);

	endTime = ReadFloat(addr + Offsets::BuffEntryBuffEndTime);
	if (endTime < *GameTime)
		return;

	int buff = ReadInt(addr + Offsets::BuffEntryBuff);
	if (CantRead(buff))
		return;
	
	startTime = ReadFloat(addr + Offsets::BuffEntryBuffStartTime);
	count     = ReadInt(addr + Offsets::BuffEntryBuffCount);
	name      = Memory::ReadString(buff + Offsets::BuffName, 100);
}

void GameBuff::ImGuiDraw()
{
	ImGui::TextColored(Color::YELLOW, name.c_str());
	ImGui::DragFloat("Start Time", &startTime);
	ImGui::DragFloat("End Time", &endTime);
	ImGui::DragInt("Count", &count);
}
