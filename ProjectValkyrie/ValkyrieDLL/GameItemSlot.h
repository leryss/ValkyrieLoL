#pragma once
#include "ItemInfo.h"
#include "GameSpell.h"

class GameItemSlot {

public:
	object     GetItemPy();
	object     GetActivePy();

	short      charges;
	ItemInfo*  item;
	GameSpell* active;
};