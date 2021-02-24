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
	attackRange   = ReadFloat(addr + Offsets::ObjAtkRange);

	targetable    = ReadBool(addr + Offsets::ObjTargetable);
	invulnerable  = ReadBool(addr + Offsets::ObjInvulnerable);
	isDead        = ReadInt(addr + Offsets::ObjSpawnCount) % 2;
	lvl           = ReadInt(addr + Offsets::ObjLvl);

	/// Read spell being cast
	int activeSpellPtr = ReadInt(addr + Offsets::ObjSpellBook + Offsets::SpellBookActiveSpellCast);
	if (activeSpellPtr != 0) {
		hasCastingSpell = true;
		castingSpell.ReadFromBaseAddress(activeSpellPtr);
	}
	else
		hasCastingSpell = false;

	/// If the unit is transformed (ex: nidalee cougar form) we read the name of the transformed unit
	bool transformFlag = (ReadInt(addr + Offsets::ObjTransformCount) % 2);
	if (transformFlag) {
		if (!isTransformed) {
			int transformation = ReadInt(addr + Offsets::ObjTransformation);
			if (transformation != NULL) {
				nameTransformed = Memory::ReadString(ReadInt(transformation));
				nameTransformed = Strings::ToLower(nameTransformed);
				staticData = GameData::GetUnit(nameTransformed);

				isTransformed = true;
			}
		}
	}
	else if (isTransformed) {
		staticData = GameData::GetUnit(name);
		isTransformed = false;
	}
}

void GameUnit::ImGuiDraw()
{
	GameObject::ImGuiDraw();
	ImGui::Separator();

	ImGui::LabelText("Transformed name", nameTransformed.c_str());
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
	ImGui::Checkbox("Targetable",     &targetable);
	ImGui::Checkbox("Invulnerable",   &invulnerable);
	ImGui::Checkbox("IsTransformed", &isTransformed);
	ImGui::DragInt("Level",           &lvl);

	ImGui::Separator();
	if(hasCastingSpell)
		castingSpell.ImGuiDraw();

	ImGui::Separator();
	if (ImGui::TreeNode("Static Data")) {
		staticData->ImGuiDraw();
		ImGui::TreePop();
	}
}

bool GameUnit::HasTags(UnitTag tag)
{
	if (staticData == nullptr)
		return false;

	return staticData->tags.test(tag);
}

float GameUnit::GetAttackSpeed()
{
	if (staticData != nullptr)
		return staticData->baseAttackSpeed * atkSpeedMulti;
	return 0.0f;
}

object GameUnit::GetStaticData()
{
	return object(ptr(staticData));
}

object GameUnit::GetCastingSpell()
{
	if (hasCastingSpell)
		return object(boost::ref(castingSpell));
	else
		return object();
}
