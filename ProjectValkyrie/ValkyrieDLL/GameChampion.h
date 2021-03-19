#pragma once
#include "GameUnit.h"
#include "GameSpell.h"
#include "GameBuff.h"
#include "GameItemSlot.h"
#include "ItemInfo.h"

#include <set>
#include <boost/python.hpp>

using namespace boost::python;

class GameChampion : public GameUnit {
	
public:
	          GameChampion();
	          GameChampion(std::string name);
		      
	void      ReadSpells(int numToRead);
	void      ReadBuffs();
	void      ReadItems();
	void      ReadFromBaseAddress(int addr);
	void      ImGuiDraw();
			  
	Vector2   GetHpBarPosition() const;
			  
	list      BuffsToPy();
	object    SpellsToPy();
	object    ItemsToPy();

	bool      HasBuff(const char* buff);
	int       BuffStackCount(const char* buff);
	bool      IsClone() const;

public:
	bool      recalling;

	const static int NUM_SPELLS = 13;
	const static int NUM_ITEMS = 7;
	GameSpell    spells[NUM_SPELLS];
	GameItemSlot items[NUM_ITEMS];

	std::map<std::string, std::shared_ptr<GameBuff>> buffs;
};