#include "GameBuff.h"
#include "imgui/imgui.h"
#include "Color.h"

void GameBuff::ReadFromBaseAddress(int addr)
{
	static float* GameTime = (float*)((int)GetModuleHandle(NULL) + Offsets::GameTime);
	
	address = addr;
	endTime = ReadFloat(addr + Offsets::BuffEntryBuffEndTime);
	if (endTime < *GameTime)
		return;

	int buff = ReadInt(addr + Offsets::BuffEntryBuff);
	if (CantRead(buff))
		return;
	
	name = Memory::ReadString(buff + Offsets::BuffName, 100);
	startTime = ReadFloat(addr + Offsets::BuffEntryBuffStartTime);
	count     = ReadInt(addr + Offsets::BuffEntryBuffCount);
	if (count == 0) {
		int start = ReadInt(addr + Offsets::BuffEntryBuffNodeStart);
		int current = ReadInt(addr + Offsets::BuffEntryBuffNodeCurrent);
		
		count = (current - start) / 0x8;
	}
}

void GameBuff::ImGuiDraw()
{
	ImGui::TextColored(Color::YELLOW, name.c_str());
	ImGui::DragInt("Address", &address, 1.f, 0, 0, "%#10x");
	ImGui::DragFloat("Start Time", &startTime);
	ImGui::DragFloat("End Time", &endTime);
	ImGui::DragInt("Count", &count);
}
