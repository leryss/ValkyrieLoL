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
#include "Collidable.h"

enum GameObjectType {

	OBJ_UNKNOWN,
	OBJ_CHAMPION,
	OBJ_MINION,
	OBJ_JUNGLE,
	OBJ_TURRET,
	OBJ_MISSILE
};

class GameObject : public MemoryReadable {

public:
	               GameObject();
	               GameObject(std::string name);
	void           ReadFromBaseAddress(int baseAddr);

	/// Checks if two game objects are allies
	bool           IsAllyTo(const GameObject& other);

	/// Checks if two game objects are enemies
	bool           IsEnemyTo(const GameObject& other);

	/// Checks if two game objects are equal by checking their network id
	bool           EqualsTo(const GameObject& other);

	bool           InFrontOf(const GameObject& other);

	virtual void   ImGuiDraw();

public:
	GameObjectType type;
	std::string    name;
	int            address;
			       
	int            networkId;
	short          index;
	short          team;
	Vector3        pos;
	Vector3        dir;
	bool           isMoving;
	bool           isVisible;

	float          firstSeen;
	float          lastSeen;
};