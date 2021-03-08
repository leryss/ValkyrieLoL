#include "SpellCast.h"
#include "Offsets.h"
#include "Memory.h"
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
	float remaining = timeBegin + castTime - Valkyrie::CurrentGameState->time;
	return (remaining < 0.f ? 0.f : remaining);
}
