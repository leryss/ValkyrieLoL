#pragma once
#include "GameState.h"
#include "boost/python.hpp"

using namespace boost::python;

enum RaycastLayer {
	RayChampion   = (1 << 0),
	RayMinion     = (1 << 1),
	RayTurret     = (1 << 2),
	RayJungle     = (1 << 3),
	RayMissile    = (1 << 4),
	RayOther      = (1 << 5),
	RayWall       = (1 << 6),
				  
	RayAlly       = (1 << 7),
	RayEnemy      = (1 << 8),
				  
	RayUnit       = RayChampion | RayMinion  | RayTurret | RayJungle,
	RayAll        = RayUnit     | RayMissile | RayOther  | RayWall
};

class RaycastResult {
public:
	Vector3 point;
	const GameObject* obj;

	object GetObjectPy();
};

class Raycast {

public:
	/// Casts a ray from a point in a given direction, if the rays hits any object specified by the RaycastLayer argument it returns the point and the object
	static std::shared_ptr<std::list<std::shared_ptr<RaycastResult>>> Cast(const GameState* state, Vector3 begin, Vector3 direction, float length, float halfWidth, bool singleResult, RaycastLayer layer);
	static RaycastLayer FindLayersFromSpell(const SpellInfo& info);
};