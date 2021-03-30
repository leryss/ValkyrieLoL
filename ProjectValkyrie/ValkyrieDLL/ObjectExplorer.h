#pragma once
#include "GameState.h"
#include "imgui/imgui.h"

/// Simple explorer UI for league's game objects
class ObjectExplorer {

public:
	static void ImGuiDraw(GameState& state);
};