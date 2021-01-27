#pragma once
#include "MemoryReadable.h"
#include <string>

class GameObject : MemoryReadable {

public:
	     GameObject(std::string name);
	void ReadFromBaseAddress(int baseAddr);

public:
	int         address;
	int         networkId;
	std::string name;

};