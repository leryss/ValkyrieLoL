#pragma once
#include "Offsets.h"
#include "Memory.h"
#include "MemoryReadable.h"

class GameBuff {

public:
	void   ImGuiDraw();

public:
	int         address;
	float       startTime;
	float       endTime;
	int         count;
	float       value;
	std::string name;
};

struct Buff {

	char pad1[0x8];
	char buffName[120];
};
	
struct BuffEntry {

	char pad1[0x8];
	Buff* buff;
	float startTime;
	float endTime;
	char pad2[0xC];
	int  buffNodeStart;
	int  buffNodeEnd; //0x28
	char pad3[0x8];
	float value;

	int GetCount() const;
};

struct BuffEntryWrap {
	BuffEntry* entry;
	char pad[4];
};