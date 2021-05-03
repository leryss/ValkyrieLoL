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
	int  buffNodeEnd;
	char pad3[0x64];
	int  count;

	int GetCount() const;
};

struct BuffEntryWrap {
	BuffEntry* entry;
	char pad[4];
};