#pragma once
#include "GameRenderer.h"
#include "GameHud.h"

struct GameState {

	float time;

	GameRenderer renderer;
	GameHud      hud;
};