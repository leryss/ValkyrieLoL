#pragma once

#include "Offsets.h"
#include <map>
#include <vector>
#include <set>

/// Performs skin changing for league champion characters
class SkinChanger {

public:

	static void   ImGuiDraw();

	/// Refreshe the skin. When a champion dies the skin is reset so this is necessarry
	static void   Refresh();

public:
	static int    CurrentSkinId;
	static int    CurrentSkinIndex;
	static int    CurrentChromaIndex;
};