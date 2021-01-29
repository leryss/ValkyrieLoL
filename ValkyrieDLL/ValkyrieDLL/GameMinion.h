#pragma once
#include "GameUnit.h"

class GameMinion : public GameUnit {

public:
	GameMinion(std::string name)
		:GameUnit(name) {
	
		type = OBJ_MINION;
	}
};