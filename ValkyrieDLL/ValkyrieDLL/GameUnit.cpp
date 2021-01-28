#include "GameUnit.h"
#include "GameData.h"

GameUnit::GameUnit()
{
}

GameUnit::GameUnit(std::string name)
	:GameObject(name)
{
	staticData = GameData::GetUnit(name);
}

void GameUnit::ReadFromBaseAddress(int addr)
{
	GameObject::ReadFromBaseAddress(addr);
	mana          = ReadFloat(addr + Offsets::ObjMana);
	health        = ReadFloat(addr + Offsets::ObjHealth);
	maxHealth     = ReadFloat(addr + Offsets::ObjMaxHealth);
	armor         = ReadFloat(addr + Offsets::ObjArmor);
	magicRes      = ReadFloat(addr + Offsets::ObjMagicRes);
	baseAtk       = ReadFloat(addr + Offsets::ObjBaseAtk);
	bonusAtk      = ReadFloat(addr + Offsets::ObjBonusAtk);
	moveSpeed     = ReadFloat(addr + Offsets::ObjMoveSpeed);
	expiry        = ReadFloat(addr + Offsets::ObjExpiry);
	crit          = ReadFloat(addr + Offsets::ObjCrit);
	critMulti     = ReadFloat(addr + Offsets::ObjCritMulti);
	abilityPower  = ReadFloat(addr + Offsets::ObjAbilityPower);
	atkSpeedMulti = ReadFloat(addr + Offsets::ObjAtkSpeedMulti);
	attackRange   = ReadFloat(addr + 0x12B8);

	isDead        = ReadInt(addr + Offsets::ObjSpawnCount) % 2;
	lvl           = ReadInt(addr + Offsets::ObjLvl);
}

void GameUnit::ImGuiDraw()
{
	GameObject::ImGuiDraw();
	ImGui::Separator();

	ImGui::DragFloat("Mana",          &mana);
	ImGui::DragFloat("Health",        &health);
	ImGui::DragFloat("MaxHealth",     &maxHealth);
	ImGui::DragFloat("Armor",         &armor);
	ImGui::DragFloat("MagicRes",      &magicRes);
	ImGui::DragFloat("BaseAtk",       &baseAtk);
	ImGui::DragFloat("BonusAtk",      &bonusAtk);
	ImGui::DragFloat("MoveSpeed",     &moveSpeed);
	ImGui::DragFloat("Expiry",        &expiry);
	ImGui::DragFloat("Crit",          &crit);
	ImGui::DragFloat("CritMulti",     &critMulti);
	ImGui::DragFloat("AbilityPower",  &abilityPower);
	ImGui::DragFloat("AtkSpeedMulti", &atkSpeedMulti);
	ImGui::DragFloat("AttackRange",   &attackRange);

	ImGui::Checkbox("IsDead",         &isDead);
	ImGui::DragInt("Level",           &lvl);

	ImGui::Separator();
	if (ImGui::TreeNode("Static Data")) {
		staticData->ImGuiDraw();
		ImGui::TreePop();
	}
}
