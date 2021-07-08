


#include "GameData.h"
#include "Valkyrie.h"

void SpellCast::ReadFromBaseAddress(int address)
{
	name.clear();
	int spellInfo = ReadInt(address + Offsets::SpellCastSpellInfo);
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);
	int nameAddr  = ReadInt(spellData + Offsets::SpellDataSpellName);

	name = Memory::ReadString(nameAddr);
	name = Strings::ToLower(name);
	staticData = GameData::GetSpell(name);

	memcpy(&start, AsPtr(address + Offsets::SpellCastStart), sizeof(Vector3));
	memcpy(&end,   AsPtr(address + Offsets::SpellCastEnd), sizeof(Vector3));
	dir = Vector3(end.x - start.x, 0.f, end.z - start.z).normalize();

	if (staticData != nullptr) {
		if (staticData->HasFlag(SpellFlags::CastDirection)) {
			end = dir.scale(staticData->castRange).add(start);
			end.y = start.y;
		}
	}

	timeBegin  = ReadFloat(address + Offsets::SpellCastStartTime);
	if (timeBegin == 0.0)
		timeBegin = ReadFloat(address + Offsets::SpellCastStartTimeAlt);
	castTime   = ReadFloat(address + Offsets::SpellCastCastTime);
	srcIndex   = ReadShort(address + Offsets::SpellCastSrcIdx);
	int addrDestIdx = ReadInt(address + Offsets::SpellCastDestIdx);
	if (CantRead(addrDestIdx))
		destIndex = 0;
	else
		destIndex  = ReadShort(addrDestIdx);
}

void SpellCast::ImGuiDraw()
{
	ImGui::Text(name.c_str());
	start.ImGuiDraw("Start Position");
	end.ImGuiDraw("End Position");
	dir.ImGuiDraw("Direction");

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

float SpellCast::RemainingCastTime() const
{
	float remaining = castTime - (Valkyrie::CurrentGameState->time - timeBegin);
	return (remaining < 0.f ? 0.f : remaining);
}
