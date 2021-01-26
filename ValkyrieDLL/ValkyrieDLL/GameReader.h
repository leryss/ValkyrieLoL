#pragma once
#include "GameState.h"

class GameReader {

public:
	GameState& GetNextState();

private:
	GameState state;
	int       baseAddr;
};