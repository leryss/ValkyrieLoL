#pragma once
#include "GameUnit.h"
#include "GameSpell.h"
#include "ItemInfo.h"

#include <set>
#include <boost/python.hpp>

using namespace boost::python;

class GameChampion : public GameUnit {
	
public:
	          GameChampion();
	          GameChampion(std::string name);
		      
	void      ReadBuffs(int addr);
	void      ReadFromBaseAddress(int addr);
	void      ImGuiDraw();
			  
	Vector2   GetHpBarPosition();
			  
	object    SpellsToPy();
	object    ItemsToPy();
	bool      HasBuff(const char* buff);

public:
	bool      recalling;
	GameSpell spells[6];
	ItemInfo* items[6];

	std::set<std::string> buffs;
};