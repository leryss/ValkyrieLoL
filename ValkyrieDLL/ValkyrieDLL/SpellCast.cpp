#include "SpellCast.h"
#include "Offsets.h"
#include "Memory.h"
#include "GameData.h"

void SpellCast::ReadFromBaseAddress(int address)
{
	int spellInfo = ReadInt(address + Offsets::SpellCastSpellInfo);
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);
	int nameAddr  = ReadInt(spellData + Offsets::SpellDataSpellName);
	name = Memory::ReadString(nameAddr);
	name = Strings::ToLower(name);
	staticData = GameData::GetSpell(name);

	memcpy(&start, AsPtr(address + Offsets::SpellCastStart), sizeof(Vector3));
	memcpy(&end,   AsPtr(address + Offsets::SpellCastEnd), sizeof(Vector3));

	srcIndex   = ReadShort(address + Offsets::SpellCastSrcIdx);
	destIndex  = ReadShort(address + Offsets::SpellCastDestIdx);
	timeBegin  = ReadFloat(address + Offsets::SpellCastStartTime);
	castTime   = ReadFloat(address + Offsets::SpellCastCastTime);
}

void SpellCast::ImGuiDraw()
{
	ImGui::Text(name.c_str());
	start.ImGuiDraw("Start Position");
	end.ImGuiDraw("End Position");

	int srcIdx = srcIndex;
	int destIdx = destIndex;
	ImGui::DragInt("Src Index", &srcIdx);
	ImGui::DragInt("Dest Index", &destIdx);
	ImGui::DragFloat("Time Begin", &timeBegin);
	ImGui::DragFloat("Cast Time", &castTime);

	ImGui::Separator();
	if(staticData != nullptr)
		staticData->ImGuiDraw();
}

object SpellCast::GetStaticData()
{
	return object(ptr(staticData));
}
