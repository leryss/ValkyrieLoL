#pragma once
#include "MemoryReadable.h"
#include "Vector.h"

class GameHud : public MemoryReadable {

public:
	void ReadFromBaseAddress(int baseAddr);

	Vector2 minimapSize;
	Vector2 minimapPosition;
};