#pragma once
#include "MemoryReadable.h"

class GameRenderer: MemoryReadable{

public:
	int width;
	int height;

	float viewMatrix[16];
	float projMatrix[16];
	float viewProjMatrix[16];

	void ReadFromBaseAddress(int baseAddr);

private:
	void     MultiplyMatrices(float *out, float *a, int row1, int col1, float *b, int row2, int col2);
};