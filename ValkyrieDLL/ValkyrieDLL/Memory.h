#pragma once

#define ReadInt(addr) *(int*)(addr)
#define ReadFloat(addr) *(float*)(addr)
#define ReadShort(addr) *(short*)(addr)
#define ReadBool(addr) *(bool*)(addr)
#define AsPtr(addr) (void*)(addr)