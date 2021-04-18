#include "Raycast.h"
#include "GameData.h"

#define RayGetNearby(fromLayer, from)\
if ((layer & fromLayer) == fromLayer) {\
	for (auto& m : from){\
		if (m->isVisible && !m->isDead && m->pos.distance(begin) < length)\
			if((allyTeam == m->team && rayAllies) || (allyTeam != m->team && rayEnemies))\
				objs.push_back(m.get());\
		}\
}\

std::shared_ptr<RaycastResult> Raycast::Cast(const GameState * state, Vector3 begin, Vector3 direction, float length, float halfWidth, RaycastLayer layer)
{
	static float distanceStep = 20.f;

	std::vector<GameObject*> objs;

	bool rayWall    = ((RayWall & layer) == RayWall);
	bool rayAllies  = ((RayAlly & layer) == RayAlly);
	bool rayEnemies = ((RayEnemy & layer) == RayEnemy);
	short allyTeam = state->player->team;

	RayGetNearby(RayMinion,   state->minions);
	RayGetNearby(RayJungle,   state->jungle);
	RayGetNearby(RayTurret,   state->turrets);
	RayGetNearby(RayMissile,  state->missiles);
	RayGetNearby(RayOther,    state->others);
	RayGetNearby(RayChampion, state->champions);

	int iters = length / distanceStep;
	float currentDist = 0.f;

	RaycastResult* result;
	for (int i = 0; i < iters; ++i) {

		Vector3 pos = begin.add(direction.scale(currentDist));
		if (rayWall && GameData::IsWallAt(pos)) {
			result = new RaycastResult();
			result->point = pos;
			return std::shared_ptr<RaycastResult>(result);
		}

		for (GameObject* obj : objs) {
			if (pos.distance(obj->pos) < (obj->GetRadius() + halfWidth)) {
				result = new RaycastResult();
				result->obj = obj;
				result->point = pos;
				return std::shared_ptr<RaycastResult>(result);
			}
		}

		currentDist += distanceStep;
	}

	return nullptr;
}

object RaycastResult::GetObjectPy()
{
	return object(ptr(obj));
}
