#pragma once
#include "GameObject.h"
#include "UnitInfo.h"

class GameUnit : public GameObject {

public:
	     GameUnit();
	     GameUnit(std::string name);

	void ReadFromBaseAddress(int addr);
	void ImGuiDraw();

public:

	bool  isDead;
	float mana;
	float health;
	float maxHealth;
	float armor;
	float magicRes;
	float baseAtk;
	float bonusAtk;
	float moveSpeed;
	int   lvl;
	float expiry;
	float crit;
	float critMulti;
	float abilityPower;
	float atkSpeedMulti;
	float attackRange;

	UnitInfo* staticData;
};