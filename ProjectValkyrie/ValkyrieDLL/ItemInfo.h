#pragma once
#include <map>

struct ItemInfo {

public:
	int id;
	float cost;
	float movementSpeed;
	float health;
	float crit;
	float abilityPower;
	float mana;
	float armour;
	float magicResist;
	float physicalDamage;
	float attackSpeed;
	float lifeSteal;
	float hpRegen;
	float movementSpeedPercent;

	/// Not a static field, but we arent going to make a whole new class just for one field are we ?
	int charges;
};
