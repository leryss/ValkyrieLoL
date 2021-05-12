#include "GameSpell.h"
#include "GameData.h"
#include "Strings.h"
#include "Valkyrie.h"
#include "Debug.h"

std::map<std::string, SummonerType> SummonerSpell::StringToType = {
	 {"s5_summonersmiteduel",                     SummonerSmite},
	 {"s5_summonersmiteplayerganker",             SummonerSmite},
	 {"summonersmite",                            SummonerSmite},
	 {"summonerdot",                              SummonerIgnite},
	 {"summonerboost",                            SummonerCleanse},
	 {"summonerteleport",                         SummonerTeleport},
	 {"summonerflash",                            SummonerFlash},
	 {"summonerflashperkshextechflashtraptionv2", SummonerHexFlash},
	 {"summonersnowball",                         SummonerSnowball},
	 {"summonermana",                             SummonerClarity},
	 {"summonerexhaust",                          SummonerExhaust},
	 {"summonerbarrier",                          SummonerBarrier},
	 {"summonerheal",                             SummonerHeal},
	 {"summonerhaste",                            SummonerGhost}
};

void GameSpell::ReadFromBaseAddress(int addr)
{
	lvl = ReadInt(addr + Offsets::SpellSlotLevel);
	readyAt = ReadFloat(addr + Offsets::SpellSlotTime);
	value = ReadFloat(addr + Offsets::SpellSlotValue);
	charges = ReadShort(addr + Offsets::SpellSlotCharges);
	readyAtCharge = ReadFloat(addr + Offsets::SpellSlotTimeCharge);

	int spellInfo = ReadInt(addr + Offsets::SpellSlotSpellInfo);
	if (CantRead(spellInfo)) {
		return;
	}
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);
	if (CantRead(spellData)) {
		return;
	}

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
	ImGui::Checkbox("Castable",         &castableBit);
}

float GameSpell::GetRemainingCooldown() const
{
	float time = Valkyrie::CurrentGameState->time;
	float cd = 0.0f;
	if (readyAt > readyAtCharge)
		cd = readyAt - time;
	else if(charges == 0) {
		cd = readyAtCharge - time;
	}
	return (cd >= 0.f ? cd : 0.0f);
}

object GameSpell::GetStaticData()
{
	return object(ptr(staticData));
}

void SummonerSpell::ReadFromBaseAddress(int addr)
{
	GameSpell::ReadFromBaseAddress(addr);

	auto find = StringToType.find(name);
	if (find == StringToType.end())
		type = SummonerUnknown;
	else
		type = find->second;
}
