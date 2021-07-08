#include "GameMinion.h"
#include "Valkyrie.h"


GameMinion::GameMinion()
{
}

Vector2 GameMinion::GetHpBarPosition()
{
	Vector3 p = pos.clone();
	p.y += staticData->healthBarHeight;

	Vector2 w2s = Valkyrie::CurrentGameState->renderer.WorldToScreen(p);
	w2s.y -= (Valkyrie::CurrentGameState->renderer.height * 0.00083333335f * staticData->healthBarHeight);
	w2s.x -= 32.f;
	return w2s;
}

void GameMinion::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);
	ReadAiManager();
}