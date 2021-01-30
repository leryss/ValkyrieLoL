#pragma once
#include "GameUnit.h"
#include "GameSpell.h"
#include "ItemInfo.h"

#include <boost/python.hpp>

using namespace boost::python;

class GameChampion : public GameUnit {
	
public:
	          GameChampion();
	          GameChampion(std::string name);
		      
	void      ReadFromBaseAddress(int addr);
	void      ImGuiDraw();
			  
	Vector2   GetHpBarPosition();
			  
	object    SpellsToPy();
	object    ItemsToPy();

public:
	GameSpell spells[6];
	ItemInfo* items[6];
};