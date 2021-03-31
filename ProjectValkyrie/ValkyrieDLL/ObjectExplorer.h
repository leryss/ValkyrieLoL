#pragma once
#include "GameState.h"
#include "imgui/imgui.h"

/// Simple explorer UI for league's game objects
class ObjectExplorer {

public:
	static void ImGuiDraw(GameState& state);

private:
	static void DrawObjects(GameState& state);
	static void DrawOffsetAdjuster();
	static void DrawOffset(const char* label, int& offset);
};