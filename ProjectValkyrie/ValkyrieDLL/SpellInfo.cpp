#include "SpellInfo.h"
#include "imgui/imgui.h"

std::map<std::string, SpellFlags> SpellInfo::FlagMap = {
	{ "CastPoint",       CastPoint },
	{ "CastAnywhere",    CastAnywhere },
	{ "CastTarget",      CastTarget },
	{ "CastDirection",   CastDirection },
	{ "CastSelf",        CastSelf },
					     
	{ "Line",            TypeLine },
	{ "Area",            TypeArea },
	{ "Cone",            TypeCone },
	{ "Rect",            TypeRect },

	{ "CollideWindwall", CollideWindwall },
	{ "CollideMinion",   CollideMinion },
	{ "CollideChampion", CollideChampion },
	{ "CollideMonster",  CollideMonster },

	{ "AffectMinion",    AffectMinion },
	{ "AffectChampion",  AffectChampion },
	{ "AffectMonster",   AffectMonster },
						 
	{ "CollideCommon",   CollideCommon },
	{ "AffectAll",       AffectAllUnits },
					     
	{ "Dash",            DashSkill },
	{ "Channel",         ChannelSkill},

};

void SpellInfo::AddFlag(std::string & flag)
{
	auto find = FlagMap.find(flag);
	if (find != FlagMap.end()) {
		flags = (SpellFlags)(flags | find->second);
	}
}

bool SpellInfo::HasFlag(SpellFlags flag) const
{
	return (flags & flag) == flag;
}

SpellFlags SpellInfo::GetSpellType() const
{
	return (SpellFlags)(flags & SpellAllTypes);
}

object SpellInfo::GetParentPy()
{
	return object(ptr(parent));
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
