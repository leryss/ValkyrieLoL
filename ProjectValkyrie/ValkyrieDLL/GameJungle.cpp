#include "GameJungle.h"

GameJungle::GameJungle()
{
}

void GameJungle::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);
	ReadAiManager();
}