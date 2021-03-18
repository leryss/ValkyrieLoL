#include "GameSpell.h"
#include "GameData.h"
#include "Strings.h"
#include "Valkyrie.h"

void GameSpell::ReadFromBaseAddress(int addr)
{
	lvl           = ReadInt(addr + Offsets::SpellSlotLevel);
	readyAt       = ReadFloat(addr + Offsets::SpellSlotTime);
	value         = ReadFloat(addr + Offsets::SpellSlotValue);
	charges       = ReadShort(addr + Offsets::SpellSlotCharges);
	readyAtCharge = ReadFloat(addr + Offsets::SpellSlotTimeCharge);

	int spellInfo = ReadInt(addr + Offsets::SpellSlotSpellInfo);
	if (CantRead(spellInfo))
		return;
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);
	if (CantRead(spellData))
		return;

	if (lvl > 0 && lvl < 6)
		mana = ReadFloat(spellData + Offsets::SpellDataManaArray + sizeof(float)*(lvl - 1));

	name = Memory::ReadString(ReadInt(spellData + Offsets::SpellDataSpellName));
	name = Strings::ToLower(name);
	staticData = GameData::GetSpell(name);
}

void GameSpell::ImGuiDraw()
{
	ImGui::Text("Name: %s",             name.c_str());
	ImGui::DragInt("Level",             &lvl);
	ImGui::DragFloat("Ready At",        &readyAt);
	ImGui::DragFloat("Charge Ready At", &readyAtCharge);
	ImGui::DragInt("Charges",           &charges);
	ImGui::DragFloat("Value",           &value);
	ImGui::DragFloat("Mana",            &mana);
}

float GameSpell::GetRemainingCooldown()
{
	float time = Valkyrie::CurrentGameState->time;
	float cd = 0.0f;
	if (readyAtCharge == 0 || charges > 0)
		cd = readyAt - time;
	else
		cd = readyAtCharge - time;
	return (cd >= 0.f ? cd : 0.0f);
}

object GameSpell::GetStaticData()
{
	return object(ptr(staticData));
}
