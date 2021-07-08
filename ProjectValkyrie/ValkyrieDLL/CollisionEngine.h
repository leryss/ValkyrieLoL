#pragma once
#include "GameState.h"


using namespace boost::python;

class FutureCollision {

public:
	FutureCollision();
	FutureCollision(GameUnit* unit, const SpellCast* cast, const Vector2& unitColPt, const Vector2& castColPt, bool isFinal, float timeUntilImpact);

	float            timeUntilImpact;
	bool             isFinal = false;
	Vector2          unitCollisionPoint;
	Vector2          castCollisionPoint;
	GameUnit*  unit;
	const SpellCast* cast;

	object GetUnitPy();
	object GetCastPy();
};

class CollisionEngine {

public:

	void Update(const GameState& state);

	list GetCollisionsForUnit(const Collidable* unit);
	list GetCollisionsForCast(const Collidable* cast);
	bool PredictPointForCollision(GameUnit& caster, GameUnit& obj, const SpellInfo& spell, Vector3& out);

	BenchmarkTiming updateTimeMs;

private:

	float   GetSecsUntilSpellHits(GameUnit& caster, GameUnit& target, const SpellInfo& spell);
	void    AddCollisionEntry(FutureCollision* collision);
	Vector2 LinearCollision(const Vector2& p1, const Vector2& d1, const Vector2& p2, const Vector2& d2, float radius);

	void    FindCollisions(const GameState* state, const GameObject& spawner, const SpellCast* cast, const SpellInfo* castStatic);
	void    FindCollisionsLine(const Vector3& spell_start, const SpellCast* cast, const SpellInfo* castStatic, std::vector<std::pair<GameUnit*, bool>>& objects);
	void    FindCollisionsArea(const Vector3& spell_start, const SpellCast* cast, const SpellInfo* castStatic, std::vector<std::pair<GameUnit*, bool>>& objects);
	void    GetNearbyEnemies(const GameState& state, const GameObject& center, const SpellInfo* spell, float distance, std::vector<std::pair<GameUnit*, bool>>& result);

	bool    PredictPointForAreaCollision(GameUnit& caster, GameUnit& obj, const SpellInfo& spell, Vector3& out);
	bool    PredictPointForConeCollision(GameUnit& caster, GameUnit& obj, const SpellInfo& spell, Vector3& out);
	bool    PredictPointForLineCollision(GameUnit& caster, GameUnit& obj, const SpellInfo& spell, Vector3& out);
private:
	
	/// Key: addr of Object/SpellCast  Value: Collisions
	std::map<const Collidable*, std::vector<std::shared_ptr<FutureCollision>>> collisions;

	const GameState* state;
};