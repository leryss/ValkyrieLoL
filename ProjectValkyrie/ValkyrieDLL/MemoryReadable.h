#pragma once

class MemoryReadable {

public:
	virtual void ReadFromBaseAddress(int address) = 0;
};