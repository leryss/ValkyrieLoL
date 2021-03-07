#pragma once
#include "GameState.h"
#include "Benchmark.h"
#include <boost/python.hpp>
using namespace boost::python;

class FutureCollision {

public:
	FutureCollision();
	FutureCollision(const GameUnit* unit, const SpellCast* cast, const Vector2& time, bool isFinal);

	bool       isFinal = false;
	Vector3    time;
	const GameUnit*  unit;
	const SpellCast* cast;

	object GetUnitPy();
	object GetCastPy();
};

class CollisionEngine {

public:

	void Update(const GameState& state);

	list GetCollisionsForUnit(const Collidable* unit);
	list GetCollisionsForCast(const Collidable* cast);

	BenchmarkTiming updateTimeMs;

private:

	void    AddCollisionEntry(FutureCollision* collision);
	Vector2 LinearCollision(const Vector2& p1, const Vector2& d1, const Vector2& p2, const Vector2& d2, float radius);
	void    FindCollisions(const Vector3& spell_start, const SpellCast* cast, const SpellInfo* castStatic, std::vector<std::pair<const GameUnit*, bool>>& objects);
	void    GetObjectsNearbyEnemies(const GameState& state, const GameObject& center, const SpellInfo* spell, float distance, std::vector<std::pair<const GameUnit*, bool>>& result);
private:
	
	/// Key: addr of Object/SpellCast  Value: Collisions
	std::map<const Collidable*, std::vector<std::shared_ptr<FutureCollision>>> collisions;
};