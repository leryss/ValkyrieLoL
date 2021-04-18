#include "GameMissile.h"

GameMissile::GameMissile()
{
}

GameMissile::GameMissile(std::string name)
	:GameObject(name)
{
	spell.staticData = GameData::GetSpell(name);
	type = OBJ_MISSILE;
}

void GameMissile::ReadFromBaseAddress(int addr)
{
	GameObject::ReadFromBaseAddress(addr);
	spell.ReadFromBaseAddress(addr + Offsets::ObjMissileSpellCast);
}

void GameMissile::ImGuiDraw()
{
	GameObject::ImGuiDraw();
	ImGui::Separator();
	spell.ImGuiDraw();
}

float GameMissile::GetRadius()
{
	if (spell.staticData == nullptr)
		return 0.f;

	return spell.staticData->width;
}

object GameMissile::GetSpell()
{
	return object(boost::ref(spell));
}
