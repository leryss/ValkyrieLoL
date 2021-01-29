#pragma once
#include "GameUnit.h"

class GameTurret : public GameUnit{

public:
	GameTurret(std::string name) 
	:GameUnit(name) {
		
		type = OBJ_TURRET;
	}
};