#pragma once

#include <string>
#include <windows.h>

#define ReadInt(addr) *(int*)(addr)
#define ReadFloat(addr) *(float*)(addr)
#define ReadShort(addr) *(short*)(addr)
#define ReadBool(addr) *(bool*)(addr)
#define AsPtr(addr) (void*)(addr)
#define CantRead(addr) IsBadReadPtr((void*)(addr), 1)

class Memory {

public:
	static std::string ReadString(int addr, int maxSize = 50);
};