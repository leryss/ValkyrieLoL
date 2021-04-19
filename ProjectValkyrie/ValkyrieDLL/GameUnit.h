#pragma once
#include "GameObject.h"
#include "UnitInfo.h"
#include "SpellCast.h"

#include <boost/python.hpp>
using namespace boost::python;

class GameUnit : public GameObject, public Collidable {

public:
	          GameUnit();
	          GameUnit(std::string name);
		      
	void      ReadFromBaseAddress(int addr);
	void      ImGuiDraw() override;
	float     GetRadius() override;

	bool      HasTags(UnitTag tag);

	float     GetAttackSpeed();
	float     GetCooldownReduction();
	float     GetAttackDamage();
	float     GetBonusMoveSpeed();
	float     GetBonusAttackSpeed();

	/// Get the physical damage after armor/lethality modifiers have been applied
	float     EffectivePhysicalDamage(const GameUnit& target, float dmg) const;

	/// Get the magical damage after magic armor/magic pen modifiers have been applied
	float     EffectiveMagicalDamage(const GameUnit target, float dmg) const;

	/// Check if the unit is a ranged unit
	bool      IsRanged();

	object    GetStaticData();
	object    GetCastingSpell();
	object    GetPathPy();

	/// Check if unit has a named buff
	bool      HasBuff(const char* buff);

	/// Check the number of stacks the named buff has
	int       BuffStackCount(const char* buff);

	/// Predicts position of this unit in N seconds
	Vector3   PredictPosition(float millisFuture) const;

	/// Calculates path length;
	float     CalculatePathLength();

	/// Changes skin of the unit
	void      Reskin(int id);

protected:
	void       ReadAiManager();

public:

	bool        isCasting = false;
	bool        targetable;
	bool        invulnerable;
	float       mana;
	float       health;
	float       maxHealth;
	float       armor;
	float       bonusArmor;
	float       magicRes;
	float       bonusMagicRes;
	float       baseAtk;
	float       bonusAtk;
	float       moveSpeed;
	int         lvl;
	float       expiry;
	float       crit;
	float       critMulti;
	float       abilityPower;
	float       atkSpeedMulti;
	float       attackRange;
	float       lethality;
	float       haste;

	float       magicPen;
	float       magicPenMulti;
	
	SpellCast   castingSpell;
	UnitInfo*   staticData;
	SpellInfo*  basicAttack;

	int         aiManagerAddress;
	bool        isMoving;
	bool        isDashing;
	float       dashSpeed;
	Vector3     destination;
	int         pathSize;
	Vector3     path[10];

	std::string nameTransformed;
};