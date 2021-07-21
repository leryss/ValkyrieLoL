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

std::shared_ptr<std::list<std::shared_ptr<RaycastResult>>> Raycast::Cast(const GameState * state, Vector3 begin, Vector3 direction, float length, float halfWidth, bool singleResult, RaycastLayer layer)
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

	int iters = (int)(length / distanceStep);
	float currentDist = 0.f;

	std::shared_ptr<std::list<std::shared_ptr<RaycastResult>>> results(new std::list<std::shared_ptr<RaycastResult>>);
	std::set<int> alreadyHit;
	bool wallAlternateBit = true;

	RaycastResult* result;
	for (int i = 0; i < iters; ++i) {

		Vector3 pos = begin.add(direction.scale(currentDist));
		if (rayWall && (GameData::IsWallAt(pos) == wallAlternateBit)) {
			result = new RaycastResult();
			result->point = pos;
			results->push_back(std::shared_ptr<RaycastResult>(result));
			if (singleResult)
				return results;
			wallAlternateBit = !wallAlternateBit;
		}

		for (GameObject* obj : objs) {
			auto find = alreadyHit.find(obj->networkId);
			if (find != alreadyHit.end())
				continue;

			if (pos.distance(obj->pos) < (obj->GetRadius() + halfWidth)) {
				result = new RaycastResult();
				result->obj = obj;
				result->point = pos;
				results->push_back(std::shared_ptr<RaycastResult>(result));
				if (singleResult)
					return results;
				alreadyHit.insert(obj->networkId);
			}
		}

		currentDist += distanceStep;
	}

	return results;
}

RaycastLayer Raycast::FindLayersFromSpell(const SpellInfo & info)
{
	int result = 0;

	if (info.HasFlag(SpellFlags::CollideChampion))
		result |= RayChampion;
	if (info.HasFlag(SpellFlags::CollideMinion))
		result |= RayMinion;
	if (info.HasFlag(SpellFlags::CollideMonster))
		result |= RayJungle;

	return (RaycastLayer)(result);
}

object RaycastResult::GetObjectPy()
{
	return object(ptr(obj));
}
