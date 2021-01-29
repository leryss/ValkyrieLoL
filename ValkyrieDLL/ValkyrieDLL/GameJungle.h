#pragma once

#include "GameUnit.h"

class GameJungle : public GameUnit {

public:
	GameJungle(std::string name) 
	:GameUnit(name) {
		
		type = OBJ_JUNGLE;
	}
};