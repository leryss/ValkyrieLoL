#pragma once
#include "Logger.h"
#include "GameState.h"
#include "PyImGui.h"

#include "imgui/imgui.h"
#include <boost/python.hpp>

using namespace boost::python;

class PyExecutionContext {

private:
	PyImGui     imgui;
	GameState*  state;
	ImDrawList* overlay;

public:
	object jungle;
	object missiles;
	object others;
	object champs;
	object turrets;
	object minion;

	object hovered;
	object player;

	float time;

public:

	object GetImGuiInterface();

	void SetGameState(GameState* state);
	void SetImGuiOverlay(ImDrawList* overlay);

	void Log(const char* msg);
};