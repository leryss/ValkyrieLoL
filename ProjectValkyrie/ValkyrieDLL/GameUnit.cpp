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
	bonusArmor    = ReadFloat(addr + Offsets::ObjBonusArmor);
	bonusMagicRes = ReadFloat(addr + Offsets::ObjBonusMagicRes);
	lethality     = ReadFloat(addr + Offsets::ObjLethality);
	haste         = ReadFloat(addr + Offsets::ObjAbilityHaste);

	magicPenMulti = ReadFloat(addr + Offsets::ObjMagicPenMulti);
	magicPen           = ReadFloat(addr + Offsets::ObjMagicPen);
	abilityPower       += abilityPower*ReadFloat(addr + Offsets::ObjAdditionalApMulti);
	

	targetable    = ReadBool(addr + Offsets::ObjTargetable);
	invulnerable  = ReadBool(addr + Offsets::ObjInvulnerable);
	isDead        = ReadInt(addr + Offsets::ObjSpawnCount) % 2;
	lvl           = ReadInt(addr + Offsets::ObjLvl);

	/// Read spell being cast
	int activeSpellPtr = ReadInt(addr + Offsets::ObjSpellBook + Offsets::SpellBookActiveSpellCast);
	if (activeSpellPtr != 0) {
		isCasting = true;
		castingSpell.ReadFromBaseAddress(activeSpellPtr);
	}
	else
		isCasting = false;

	/// Check if transformed (ex nidalee cougar form) and update static data
	int transformAddr = ReadInt(addr + Offsets::ObjTransformation);
	if (transformAddr != NULL) {
		nameTransformed = Memory::ReadString(ReadInt(transformAddr));
		nameTransformed = Strings::ToLower(nameTransformed);

		staticData = GameData::GetUnit(nameTransformed);
		if (staticData == nullptr) {
			staticData = GameData::GetUnit(name);
		}
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
	ImGui::DragFloat("Bonus Armor",   &bonusArmor);
	ImGui::DragFloat("MagicRes",      &magicRes);
	ImGui::DragFloat("Bonus MagicRes",&bonusMagicRes);
	ImGui::DragFloat("BaseAtk",       &baseAtk);
	ImGui::DragFloat("BonusAtk",      &bonusAtk);
	ImGui::DragFloat("MoveSpeed",     &moveSpeed);
	ImGui::DragFloat("Expiry",        &expiry);
	ImGui::DragFloat("Crit",          &crit);
	ImGui::DragFloat("CritMulti",     &critMulti);
	ImGui::DragFloat("AbilityPower",  &abilityPower);
	ImGui::DragFloat("AtkSpeedMulti", &atkSpeedMulti);
	ImGui::DragFloat("AttackRange",   &attackRange);
	ImGui::DragFloat("Lethality",     &lethality);
	ImGui::DragFloat("Haste",         &haste);
	ImGui::DragFloat("Magic Pen",     &magicPen);
	ImGui::DragFloat("Enemy MagicRes Multi", &magicPenMulti);

	ImGui::Checkbox("IsDead",         &isDead);
	ImGui::Checkbox("Targetable",     &targetable);
	ImGui::Checkbox("Invulnerable",   &invulnerable);
	ImGui::DragInt("Level",           &lvl);

	ImGui::Separator();
	if (ImGui::TreeNode("Static Data")) {
		staticData->ImGuiDraw();
		ImGui::TreePop();
	}

	ImGui::Separator();
	if (ImGui::TreeNode("Currently casting")) {
		if (isCasting)
			castingSpell.ImGuiDraw();
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
	return 0.f;
}

float GameUnit::GetAttackDamage()
{
	return baseAtk + bonusAtk;
}

float GameUnit::GetBonusMoveSpeed()
{
	if (staticData == nullptr)
		return 0.f;
	return moveSpeed - staticData->baseMovementSpeed;
}

float GameUnit::GetBonusAttackSpeed()
{
	if (staticData == nullptr)
		return 0.f;
	return GetAttackSpeed() - staticData->baseAttackSpeed;
}

float GameUnit::EffectivePhysicalDamage(const GameUnit & target, float dmg) const
{
	float flatArmorPen = lethality * (0.6f + (0.4f * lvl) / 18.f);
	float armorMulti = 100.f / (100.f + max(0.f, target.armor - flatArmorPen));

	return armorMulti * dmg;
}

float GameUnit::EffectiveMagicalDamage(const GameUnit target, float dmg) const
{
	float magicMulti = 100.f / (100.f + max(0.f, target.magicRes - magicPen)*magicPenMulti);

	return magicMulti * dmg;
}

float GameUnit::GetCooldownReduction()
{
	return 1.0f - (1.0f/(1.0f + (haste / 100.f)));
}

bool GameUnit::IsRanged()
{
	if (staticData == nullptr)
		return false;
	return staticData->baseAttackRange >= 300.f;
}

object GameUnit::GetStaticData()
{
	return object(ptr(staticData));
}

object GameUnit::GetCastingSpell()
{
	if (isCasting)
		return object(boost::ref(castingSpell));
	else
		return object();
}

bool GameUnit::HasBuff(const char * buff)
{
	return false;
}

int GameUnit::BuffStackCount(const char * buff)
{
	return 0;
}
