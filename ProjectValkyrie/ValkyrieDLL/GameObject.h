#pragma once
#include "MemoryReadable.h"
#include "Vector.h"
#include "imgui/imgui.h"
#include "Strings.h"



#include "Debug.h"

#include <string>
#include <windows.h>
#include <deque>
#include "Collidable.h"


enum GameObjectType {

	OBJ_UNKNOWN,
	OBJ_CHAMPION,
	OBJ_MINION,
	OBJ_JUNGLE,
	OBJ_TURRET,
	OBJ_MISSILE
};

/// Base league of legends game object
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

	/// Checks if the object is in front of another one. The calculations are performed at a 180 degree angle
	bool           InFrontOf(const GameObject& other);

	virtual float  GetRadius();
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
	bool           isVisible;
	bool           isDead;

	float          firstSeen;
	float          lastSeen;
};