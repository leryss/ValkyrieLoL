#pragma once
#include "GameState.h"

class GameReader {

public:
	const GameState* GetNextState();

private:
	GameState state;
};