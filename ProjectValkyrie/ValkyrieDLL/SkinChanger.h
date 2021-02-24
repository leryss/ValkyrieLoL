#pragma once

#include "Offsets.h"
#include <map>
#include <vector>
#include <set>


class SkinChanger {

public:

	static void   ImGuiDraw();
	static void   Refresh();

private:
	static int    CurrentSkinId;
	static int    CurrentSkinIndex;
	static int    CurrentChromaIndex;

	static bool   HasDied;
};