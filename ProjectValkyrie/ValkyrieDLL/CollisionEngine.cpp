#include "CollisionEngine.h"

void CollisionEngine::Update(const GameState & state)
{
	updateTimeMs.Start();

	this->state = &state;
	collisions.clear();

	/// Predict missiles
	for (auto& missile : state.missiles) {
		if (missile->spell.staticData == nullptr)
			continue;

		auto staticData = missile->spell.staticData;
		FindCollisions(&state, *(GameObject*)missile.get(), &missile->spell, staticData);
	}

	/// Predict spells being cast
	for (auto& champ : state.champions) {
		if (!champ->isCasting)
			continue;

		auto cast = &champ->castingSpell;
		if (cast->RemainingCastTime() < 0.f)
			continue;

		auto staticData = cast->staticData;
		FindCollisions(&state, *(GameObject*)champ.get(), cast, staticData);
	}

	updateTimeMs.End();
}

list CollisionEngine::GetCollisionsForUnit(const Collidable* collidable)
{
	list l;
	auto find = collisions.find(collidable);
	if (find != collisions.end()) {
		for (auto& spec : find->second) {
			l.append(ptr(spec.get()));
		}
	}

	return l;
}

list CollisionEngine::GetCollisionsForCast(const Collidable* collidable)
{
	list l;
	auto find = collisions.find(collidable);
	if (find != collisions.end()) {
		for (auto& spec : find->second) {
			l.append(ptr(spec.get()));
		}
	}

	return l;
}

void CollisionEngine::FindCollisions(const GameState* state, const GameObject & spawner, const SpellCast * cast, const SpellInfo * castStatic)
{
	if (castStatic == nullptr)
		return;

	std::vector<std::pair<const GameUnit*, bool>> targets;
	Vector3 currentPos = spawner.pos;
	currentPos.y = 0.f;

	switch (castStatic->GetSpellType()) {
		case TypeLine:
			GetNearbyEnemies(*state, spawner, castStatic, spawner.pos.distance(cast->end) + 700.f, targets);
			FindCollisionsRaytraced(currentPos, cast, castStatic, targets);
			break;
		case TypeArea:
			GetNearbyEnemies(*state, spawner, castStatic, spawner.pos.distance(cast->end) + castStatic->castRadius + 700.f, targets);
			FindCollisionsArea(currentPos, cast, castStatic, targets);
			break;
		default:
			break;
	}
}

void CollisionEngine::FindCollisionsRaytraced(const Vector3 & spell_start, const SpellCast * cast, const SpellInfo * castStatic, std::vector<std::pair<const GameUnit*, bool>>& objects)
{
	float UnitDelta = 1.0 + castStatic->width/3.f;

	Vector2 spellStart = Vector2(spell_start.x, spell_start.z);
	Vector2 spellEnd = Vector2(cast->end.x, cast->end.z);

	float distanceLeft = spellStart.distance(spellEnd);
	int   iterations   = max(0, distanceLeft - UnitDelta) / UnitDelta;
	float timePerIter  = UnitDelta / castStatic->speed;
	
	float deltaXSpell = cast->dir.x * timePerIter * castStatic->speed;
	float deltaYSpell = cast->dir.z * timePerIter * castStatic->speed;
	float castTime    = cast->RemainingCastTime() + castStatic->delay;

	for(auto& pair : objects) {
		auto& target = pair.first;
		if (target->staticData == nullptr) {
			Logger::Warn("Obj %s doesn't have staticData", target->name.c_str());
			continue;
		}

		Vector2 targetPos = Vector2(target->pos.x, target->pos.z);
		float deltaX = 0.f;
		float deltaY = 0.f;

		if (target->isMoving) {
			if (castTime > 0.f) {
				targetPos.x += target->dir.x*target->moveSpeed*castTime;
				targetPos.y += target->dir.z*target->moveSpeed*castTime;
			}

			deltaX += target->dir.x*target->moveSpeed*timePerIter;
			deltaY += target->dir.z*target->moveSpeed*timePerIter;
		}

		
		Vector2 spellPos = spellStart;

		for (int i = 0; i < iterations; ++i) {
			if (targetPos.distance(spellPos) < target->staticData->gameplayRadius + castStatic->width) {
				
				AddCollisionEntry(new FutureCollision(target, cast, targetPos, spellPos, pair.second, castTime + timePerIter*i));
				if (pair.second)
					return;
				break;
			}

			spellPos.x += deltaXSpell;
			spellPos.y += deltaYSpell;

			targetPos.x += deltaX;
			targetPos.y += deltaY;
		}
	}
}

void CollisionEngine::FindCollisionsArea(const Vector3 & spellCurrentPos, const SpellCast * cast, const SpellInfo * castStatic, std::vector<std::pair<const GameUnit*, bool>>& objects)
{
	Vector3 spellEnd = cast->end;
	Vector3 spellStart = cast->start;
	spellEnd.y = 0.f;
	spellStart.y = 0.f;

	for (auto& pair : objects) {
		auto target = pair.first;
		Vector3 targetPos = target->pos;
		targetPos.y = 0.f;

		float secsUntilSpellHits = 0.f;
		if (castStatic->delay > 0) {
			float distanceLeft = spellCurrentPos.distance(spellEnd);
			float distanceTotal = spellEnd.distance(spellStart);

			secsUntilSpellHits = (castStatic->delay * (distanceLeft / distanceTotal));
		}
		else 
			secsUntilSpellHits = spellCurrentPos.distance(spellEnd) / castStatic->speed;
		secsUntilSpellHits += cast->RemainingCastTime();
		
		Vector3 targetFuturePos = targetPos;
		targetFuturePos.y = 0.f;

		if (target->isMoving) {
			targetFuturePos = targetFuturePos.add(target->dir.scale(target->moveSpeed * secsUntilSpellHits));
		}
		
		if (targetFuturePos.distance(cast->end) < (castStatic->castRadius + target->staticData->gameplayRadius)) {
			AddCollisionEntry(new FutureCollision(target, cast, Vector2(targetFuturePos.x, targetFuturePos.z), Vector2(cast->end.x, cast->end.z), false, secsUntilSpellHits));
		}
	}
}

void CollisionEngine::GetNearbyEnemies(const GameState& state, const GameObject& center, const SpellInfo* spell, float distance, std::vector<std::pair<const GameUnit*, bool>>& result)
{
	bool strict;
	if (spell->HasFlag(AffectMinion)) {
		strict = spell->HasFlag(CollideMinion);
		for (auto& minion : state.minions)
			if (minion->targetable && !minion->isDead && minion->IsEnemyTo(center) && minion->pos.distance(center.pos) < distance)
				result.push_back({ (GameUnit*)minion.get() , strict });
	}

	if (spell->HasFlag(AffectChampion)) {
		strict = spell->HasFlag(CollideChampion);
		for (auto& champ : state.champions)
			if (champ->targetable && !champ->isDead && champ->IsEnemyTo(center) && champ->pos.distance(center.pos) < distance)
				result.push_back({ (GameUnit*)champ.get() , strict });
	}

	if (spell->HasFlag(AffectMonster)) {
		strict = spell->HasFlag(CollideMonster);
		for (auto& monster : state.jungle)
			if (monster->targetable && !monster->isDead && monster->IsEnemyTo(center) && monster->pos.distance(center.pos) < distance)
				result.push_back({ (GameUnit*)monster.get() , strict });
	}

	std::sort(result.begin(), result.end(), [&center](const std::pair<const GameUnit*, bool> p1, std::pair<const GameUnit*, bool> p2) {
		float d1 = p1.first->pos.distance(center.pos);
		float d2 = p2.first->pos.distance(center.pos);
		return d1 < d2;
	});
}

void CollisionEngine::AddCollisionEntry(FutureCollision* collision)
{
	/// Add collision for unit
	auto find = collisions.find(collision->unit);
	if (find == collisions.end()) {
		collisions[collision->unit] = { std::shared_ptr<FutureCollision>(collision) };
	}
	else {
		find->second.push_back(std::shared_ptr<FutureCollision>(collision));
	}

	/// Add collision for spell
	find = collisions.find(collision->cast);
	if (find == collisions.end()) {
		collisions[collision->cast] = { std::shared_ptr<FutureCollision>(collision) };
	}
	else {
		find->second.push_back(std::shared_ptr<FutureCollision>(collision));
	}
}

Vector2 CollisionEngine::LinearCollision(const Vector2& p1, const Vector2& d1, const Vector2& p2, const Vector2& d2, float radius) {

	static float Ax, Bx, Cx, Ay, By, Cy;
	static float a, b, c, delta;
	static float sqrt_d, t1, t2;

	Ax = pow((d1.x - d2.x), 2.f);
	Bx = p1.x*d1.x - p1.x*d2.x - p2.x*d1.x + p2.x*d2.x;
	Cx = pow((p1.x - p2.x), 2.f);

	Ay = pow((d1.y - d2.y), 2.f);
	By = p1.y*d1.y - p1.y*d2.y - p2.y*d1.y + p2.y*d2.y;
	Cy = pow((p1.y - p2.y), 2.f);

	a = Ax + Ay;
	b = 2.f*(Bx + By);
	c = Cx + Cy - pow(radius, 2.f);
	delta = b * b - 4.f*a*c;

	if (a == 0.f || delta < 0.f)
		return Vector2(-1.f, -1.f);

	sqrt_d = sqrt(delta);
	t1 = (-b + sqrt_d) / (2.f*a);
	t2 = (-b - sqrt_d) / (2.f*a);

	return Vector2(t1, t2);
}

FutureCollision::FutureCollision()
{
}

FutureCollision::FutureCollision(const GameUnit* unit, const SpellCast* cast, const Vector2& unitColPt, const Vector2& castColPt, bool isFinal, float timeUntilImpact)
	:
	unit(unit),
	cast(cast),
	unitCollisionPoint(unitColPt),
	castCollisionPoint(castColPt),
	isFinal(isFinal),
	timeUntilImpact(timeUntilImpact)
{
}

object FutureCollision::GetUnitPy()
{
	return object(ptr(unit));
}

object FutureCollision::GetCastPy()
{
	return object(ptr(cast));
}
