#pragma once
#include "Offsets.h"
#include "Memory.h"
#include "MemoryReadable.h"

class GameBuff : public MemoryReadable {

public:
	void   ReadFromBaseAddress(int addr);
	void   ImGuiDraw();

public:
	int         address;
	float       startTime;
	float       endTime;
	int         count;
	std::string name;
};