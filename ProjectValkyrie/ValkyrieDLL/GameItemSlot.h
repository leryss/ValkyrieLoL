#pragma once
#include "ItemInfo.h"
#include "GameSpell.h"
#include <boost/python.hpp>

using namespace boost::python;

class GameItemSlot {

public:
	object     GetItemPy();
	object     GetActivePy();

	short      charges;
	ItemInfo*  item;
	GameSpell* active;
};