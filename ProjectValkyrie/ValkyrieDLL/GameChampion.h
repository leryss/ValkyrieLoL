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
	void      ReadItems();
	void      ReadFromBaseAddress(int addr);
	void      ImGuiDraw();
			
	Vector2   GetHpBarPosition() const;
			 
	object    SpellsToPy();
	object    ItemsToPy();

	bool      CanCast(const GameSpell* spell);
	bool      IsClone() const;

public:
	bool      recalling;
	bool      channeling;

	const static int NUM_SPELLS = 13;
	const static int NUM_ITEMS = 7;
	GameSpell    spells[NUM_SPELLS];
	GameItemSlot items[NUM_ITEMS];

};