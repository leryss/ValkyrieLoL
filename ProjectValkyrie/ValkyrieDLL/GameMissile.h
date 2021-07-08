#pragma once
#include "GameObject.h"
#include "SpellInfo.h"
#include "GameData.h"


class GameMissile : public GameObject {

public:
	       GameMissile();
	       GameMissile(std::string name);
	void   ReadFromBaseAddress(int addr);
	void   ImGuiDraw() override;
	float  GetRadius() override;

	object GetSpell();

public:
	SpellCast  spell;
};