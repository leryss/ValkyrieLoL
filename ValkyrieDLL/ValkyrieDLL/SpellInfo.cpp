#include "SpellInfo.h"
#include "imgui/imgui.h"


SpellInfo * SpellInfo::AddFlags(SpellFlags flags)
{
	this->flags = (SpellFlags)(this->flags | flags);
	return this;
}

void SpellInfo::ImGuiDraw()
{
	ImGui::DragFloat("Cast Radius", &castRadius);
	ImGui::DragFloat("Cast Range",  &castRange);
	ImGui::DragFloat("Delay",       &delay);
	ImGui::DragFloat("Height",      &height);
	ImGui::DragFloat("Speed",       &speed);
	ImGui::DragFloat("Travel Time", &travelTime);
	ImGui::DragFloat("Width",       &width);
}
