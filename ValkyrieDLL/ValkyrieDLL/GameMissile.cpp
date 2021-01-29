#include "GameMissile.h"

GameMissile::GameMissile()
{
}

GameMissile::GameMissile(std::string name)
	:GameObject(name)
{
	staticData = GameData::GetSpell(name);
	type = OBJ_MISSILE;
}

void GameMissile::ReadFromBaseAddress(int addr)
{
	GameObject::ReadFromBaseAddress(addr);
	memcpy(&startPos, AsPtr(addr + Offsets::MissileStartPos), sizeof(Vector3));
	memcpy(&endPos,   AsPtr(addr + Offsets::MissileEndPos),   sizeof(Vector3));
	
	srcIndex  = ReadShort(addr + Offsets::MissileSrcIdx);
	destIndex = ReadShort(addr + Offsets::MissileDestIdx);
}

void GameMissile::ImGuiDraw()
{
	GameObject::ImGuiDraw();
	ImGui::Separator();
	startPos.ImGuiDraw("Start Position");
	endPos.ImGuiDraw("End Position");
	
	int srcIdx = srcIndex;
	int destIdx = destIndex;
	ImGui::DragInt("Src Index", &srcIdx);
	ImGui::DragInt("Dest Index", &destIdx);

	ImGui::Separator();
	staticData->ImGuiDraw();
}
