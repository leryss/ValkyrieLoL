#pragma once
#include "MemoryReadable.h"
#include "Vector.h"
#include "imgui/imgui.h"
#include "Strings.h"
#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"

#include <string>
#include <windows.h>

enum GameObjectType {

	OBJ_UNKNOWN,
	OBJ_CHAMPION,
	OBJ_MINION,
	OBJ_JUNGLE,
	OBJ_TURRET,
	OBJ_MISSILE
};

class GameObject : MemoryReadable {

public:
	               GameObject();
	               GameObject(std::string name);
	void           ReadFromBaseAddress(int baseAddr);
	bool           IsAllyTo(const GameObject& other);
	bool           IsEnemyTo(const GameObject& other);
	bool           EqualsTo(const GameObject& other);
	virtual void   ImGuiDraw();

public:
	GameObjectType type;
	std::string    name;
	int            address;
			       
	int            networkId;
	short          index;
	short          team;
	Vector3        pos;
	bool           isVisible;
	float          lastSeen;
};