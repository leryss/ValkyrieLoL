#pragma once
#include "Memory.h"
#include "Offsets.h"
#include "MemoryReadable.h"
#include "SpellInfo.h"
#include "imgui/imgui.h"

class GameSpell : public MemoryReadable {

public:
	void ReadFromBaseAddress(int addr);
	void ImGuiDraw();

public:
	std::string name;
	int         lvl;
	float       readyAt;
	float       value;

	SpellInfo*  staticData;
};