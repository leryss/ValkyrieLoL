#include "GameSpell.h"
#include "GameData.h"
#include "Strings.h"
#include "Valkyrie.h"

void GameSpell::ReadFromBaseAddress(int addr)
{
	lvl     = ReadInt(addr + Offsets::SpellSlotLevel);
	readyAt = ReadFloat(addr + Offsets::SpellSlotTime);
	value   = ReadFloat(addr + Offsets::SpellSlotValue);

	int spellInfo = ReadInt(addr + Offsets::SpellSlotSpellInfo);
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);

	name       = Memory::ReadString(ReadInt(spellData + Offsets::SpellDataSpellName));
	name       = Strings::ToLower(name);
	staticData = GameData::GetSpell(name);
}

void GameSpell::ImGuiDraw()
{
	ImGui::Text("Name: %s",      name.c_str());
	ImGui::DragInt("Level",      &lvl);
	ImGui::DragFloat("Ready At", &readyAt);
	ImGui::DragFloat("Value",    &value);
}

float GameSpell::GetRemainingCooldown()
{
	float cd = readyAt - Valkyrie::CurrentGameState->time;
	return (cd >= 0.f ? cd : 0.0f);
}

object GameSpell::GetStaticData()
{
	return object(ptr(staticData));
}
