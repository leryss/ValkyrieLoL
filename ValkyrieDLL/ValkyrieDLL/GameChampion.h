#pragma once
#include "GameUnit.h"
#include "GameSpell.h"
#include "ItemInfo.h"

class GameChampion : public GameUnit {
	
public:
	GameChampion(std::string name);

	void ReadFromBaseAddress(int addr);
	void ImGuiDraw();

public:

	GameSpell spells[6];
	ItemInfo* items[6];
};