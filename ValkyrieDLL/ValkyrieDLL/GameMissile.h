#pragma once
#include "GameObject.h"
#include "SpellInfo.h"
#include "GameData.h"

class GameMissile : public GameObject {

public:
	     GameMissile(std::string name);
	void ReadFromBaseAddress(int addr);
	void ImGuiDraw();

public:

	short   srcIndex;
	short   destIndex;
	Vector3 startPos;
	Vector3 endPos;

	SpellInfo* staticData;
};