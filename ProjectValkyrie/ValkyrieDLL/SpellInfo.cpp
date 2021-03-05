#include "SpellInfo.h"
#include "imgui/imgui.h"

std::map<std::string, SpellFlags> SpellInfo::FlagMap = {
	{ "CastPoint", CastPoint },
	{ "CastAnywhere", CastAnywhere },
	{ "CastTarget", CastTarget },
	{ "CastDirection", CastDirection },

	{ "TypeLine", TypeLine },
	{ "TypeArea", TypeArea },
	{ "TypeCone", TypeCone },
	{ "TypeTargeted", TypeTargeted },

	{ "CollideWindwall", CollideWindwall },
	{ "CollideMinion", CollideMinion },
	{ "CollideChampion", CollideChampion },
	{ "CollideMonster", CollideMonster },

	{ "AffectMinion", AffectMinion },
	{ "AffectChampion", AffectChampion },
	{ "AffectMonster", AffectMonster },

	{ "CollideCommon", CollideCommon },
	{ "AffectAllUnits", AffectAllUnits }
};

void SpellInfo::AddFlag(std::string & flag)
{
	auto find = FlagMap.find(flag);
	if (find != FlagMap.end()) {
		flags = (SpellFlags)(flags | find->second);
	}
}

bool SpellInfo::HasFlag(SpellFlags flag)
{
	return (flags & flag) == flag;
}

void SpellInfo::ImGuiDraw()
{
	ImGui::Text("Icon: %s",         icon.c_str());
	ImGui::DragFloat("Cast Radius", &castRadius);
	ImGui::DragFloat("Cast Range",  &castRange);
	ImGui::DragFloat("Cast Time",   &castTime);
	ImGui::DragFloat("Height",      &height);
	ImGui::DragFloat("Speed",       &speed);
	ImGui::DragFloat("Travel Time", &travelTime);
	ImGui::DragFloat("Width",       &width);
}
