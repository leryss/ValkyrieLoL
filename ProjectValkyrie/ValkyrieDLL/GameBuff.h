#pragma once
#include "Offsets.h"
#include "Memory.h"
#include "MemoryReadable.h"

class GameBuff : public MemoryReadable {

public:
	void   ReadFromBaseAddress(int addr);
	void   ImGuiDraw();

public:
	float       startTime;
	float       endTime;
	std::string name;
};