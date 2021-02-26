#pragma once
#include "GameUnit.h"
#include "GameSpell.h"
#include "GameBuff.h"
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
			  
	list      BuffsToPy();
	object    SpellsToPy();
	object    ItemsToPy();
	bool      HasBuff(const char* buff);

public:
	bool      recalling;

	const static int NUM_SPELLS = 12;
	const static int NUM_ITEMS = 6;
	GameSpell spells[NUM_SPELLS];
	ItemInfo* items[NUM_ITEMS];

	std::map<std::string, std::shared_ptr<GameBuff>> buffs;
};