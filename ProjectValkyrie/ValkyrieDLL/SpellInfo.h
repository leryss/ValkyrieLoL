#pragma once
#include <string>
#include <map>

#include <boost/python.hpp>
using namespace boost::python;

enum SpellFlags {
	
	CastPoint        = (1 << 0),
	CastAnywhere     = (1 << 1),
	CastTarget       = (1 << 2),
	CastDirection    = (1 << 3),

	TypeLine         = (1 << 4),
	TypeArea         = (1 << 5),
	TypeCone         = (1 << 6),
	TypeTargeted     = (1 << 7),
	TypeRect         = (1 << 8),

	CollideWindwall  = (1 << 12),
	CollideMinion    = (1 << 13),
	CollideChampion  = (1 << 14),
	CollideMonster   = (1 << 15),

	AffectMinion     = (1 << 16),
	AffectChampion   = (1 << 17),
	AffectMonster    = (1 << 18),

	SpellAllTypes   = TypeLine | TypeArea | TypeCone | TypeTargeted | TypeRect,
	CollideCommon   = CollideWindwall | CollideMinion | CollideChampion | CollideMonster,
	AffectAllUnits  = AffectMinion | AffectChampion | AffectMonster
};

/// Static data of a spell that we load from disk
class SpellInfo {

public:

	void       AddFlag(std::string& flag);
	bool       HasFlag(SpellFlags flag) const;
	SpellFlags GetSpellType() const;
	object GetParentPy();

	// Values from game's data files
	std::string name;
	std::string parentName;
	std::string icon;

	float castTime;
	float castRange;
	float castRadius;
	float castConeAngle;
	float castConeDistance;
	float delay;
	float width;
	float height;
	float speed;
	float travelTime;

	SpellFlags flags;
	SpellInfo* parent;

	void ImGuiDraw();

private:
	static std::map<std::string, SpellFlags> FlagMap;
};

