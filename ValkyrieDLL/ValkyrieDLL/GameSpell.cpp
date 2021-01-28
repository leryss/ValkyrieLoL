#include "GameSpell.h"
#include "GameData.h"

void GameSpell::ReadFromBaseAddress(int addr)
{
	lvl     = ReadInt(addr + Offsets::SpellSlotLevel);
	readyAt = ReadFloat(addr + Offsets::SpellSlotTime);
	value   = ReadFloat(addr + Offsets::SpellSlotValue);

	int spellInfo = ReadInt(addr + Offsets::SpellSlotSpellInfo);
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);

	name = Memory::ReadString(ReadInt(spellData + Offsets::SpellDataSpellName));
	staticData = GameData::GetSpell(name);
}

void GameSpell::ImGuiDraw()
{
	ImGui::Text("Name: %s",      name.c_str());
	ImGui::DragInt("Level",      &lvl);
	ImGui::DragFloat("Ready At", &readyAt);
	ImGui::DragFloat("Value",    &value);
}
