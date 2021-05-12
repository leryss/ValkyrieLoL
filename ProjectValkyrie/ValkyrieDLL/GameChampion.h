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

	object    GetQ();
	object    GetW();
	object    GetE();
	object    GetR();
	object    GetSummoner(SummonerType type);
	object    GetItem(int id);

	bool      HasItem(int id);
	bool      CanCast(const GameSpell* spell);
	bool      IsClone() const;

public:
	bool      recalling;
	bool      channeling;

	const static int NUM_ABILITIES = 4;
	const static int NUM_SUMMONERS = 2;
	const static int NUM_SPELLS    = 13;
	const static int NUM_ITEMS     = 7;
	GameSpell     abilities[NUM_ABILITIES];
	SummonerSpell summoners[NUM_SUMMONERS];
	GameSpell     itemSpells[NUM_ITEMS];

	list         pySpells;
	list         pyItems;
	GameItemSlot items[NUM_ITEMS];

};