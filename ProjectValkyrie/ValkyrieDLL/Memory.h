#pragma once

#include <string>
#include <windows.h>

/// Macros for reading memory pointers cleanly
#define ReadInt(addr) *(int*)(addr)
#define ReadFloat(addr) *(float*)(addr)
#define ReadShort(addr) *(short*)(addr)
#define ReadBool(addr) *(bool*)(addr)
#define ReadChar(addr) *(char*)(addr)
#define ReadVTable(addr, idx) ((int*)ReadInt(addr))[idx]
#define AsFunc(addr, ret, ...) reinterpret_cast<ret(__thiscall*)(##__VA_ARGS__)>(addr)
#define AsPtr(addr) (void*)(addr)
#define CantRead(addr) IsBadReadPtr((void*)(addr), 1)

class Memory {

public:
	/// Reads a string from the specified memory address. maxSize limits the size of the string read 
	static std::string ReadString(int addr, int maxSize = 50);
};