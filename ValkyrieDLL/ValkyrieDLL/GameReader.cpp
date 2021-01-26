#include "GameReader.h"
#include "Offsets.h"
#include <windows.h>

GameState& GameReader::GetNextState()
{
	baseAddr = (int)GetModuleHandle(NULL);
	memcpy(&state.time, (void*)(baseAddr + Offsets::GameTime), sizeof(float));
	
	if (state.time > 1.f) {
		state.renderer.ReadFromBaseAddress(baseAddr);
		state.hud.ReadFromBaseAddress(baseAddr);
	}
	return state;
}
