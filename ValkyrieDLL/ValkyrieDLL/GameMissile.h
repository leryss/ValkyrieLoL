#pragma once
#include "GameObject.h"
#include "SpellInfo.h"
#include "GameData.h"
#include "SpellCast.h"

class GameMissile : public GameObject {

public:
	       GameMissile();
	       GameMissile(std::string name);
	void   ReadFromBaseAddress(int addr);
	void   ImGuiDraw();
	object GetSpell();

public:
	SpellCast  spell;
};