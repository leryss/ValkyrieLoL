#pragma once
#include "GameRenderer.h"
#include "GameHud.h"
#include "GameObject.h"

#include <map>

struct GameState {

	float                                      time;
				                               
	GameRenderer                               renderer;
	GameHud                                    hud;
	std::map<int, std::shared_ptr<GameObject>> objectCache;
};