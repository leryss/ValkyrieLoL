#pragma once
#include <string>
#include <map>

enum SpellFlags {
	
	CastPoint        = (1 << 0),
	CastAnywhere     = (1 << 1),
	CastTarget       = (1 << 2),
	CastDirection    = (1 << 3),

	TypeLine         = (1 << 4),
	TypeArea         = (1 << 5),
	TypeCone         = (1 << 6),
	TypeTargeted     = (1 << 7),

	CollideWindwall  = (1 << 8),
	CollideMinion    = (1 << 9),
	CollideChampion  = (1 << 10),
	CollideMonster   = (1 << 11),

	AffectMinion     = (1 << 12),
	AffectChampion   = (1 << 13),
	AffectMonster    = (1 << 14),

	CollideCommon   = CollideWindwall | CollideMinion | CollideChampion | CollideMonster,
	AffectAllUnits  = AffectMinion | AffectChampion | AffectMonster
};

/// Static data of a spell that we load from disk
class SpellInfo {

public:

	void AddFlag(std::string& flag);
	bool HasFlag(SpellFlags flag);

	// Values from game's data files
	std::string name;
	std::string parent;
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

	void ImGuiDraw();

private:
	static std::map<std::string, SpellFlags> FlagMap;
};

