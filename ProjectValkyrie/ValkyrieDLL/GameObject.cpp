#include "GameObject.h"

GameObject::GameObject()
{
	this->name = std::string("Unknown");
	this->type = OBJ_UNKNOWN;
}

GameObject::GameObject(std::string name)
{
	this->name = name;
	this->type = OBJ_UNKNOWN;
}

void GameObject::ReadFromBaseAddress(int baseAddr)
{
	DBG_INFO("Reading game object %s", name.c_str());
	address   = baseAddr;
	networkId = ReadInt(baseAddr + Offsets::ObjNetworkID);

	index     = ReadShort(baseAddr + Offsets::ObjIndex);
	team      = ReadShort(baseAddr + Offsets::ObjTeam);
	isVisible = ReadBool(baseAddr + Offsets::ObjVisibility);

	memcpy(&pos, AsPtr(baseAddr + Offsets::ObjPos), sizeof(Vector3));
	memcpy(&dir, AsPtr(baseAddr + Offsets::ObjDirection), sizeof(Vector3));
}

bool GameObject::IsAllyTo(const GameObject& other)
{
	return other.team == this->team;
}

bool GameObject::IsEnemyTo(const GameObject& other)
{
	return other.team != this->team;
}

bool GameObject::EqualsTo(const GameObject& other)
{
	return this->networkId == other.networkId;
}

bool GameObject::InFrontOf(const GameObject & other)
{
	auto diff = pos.sub(other.pos);

	return diff.dot(other.dir) > 0.f;
}

void GameObject::ImGuiDraw()
{
	int idx = index;
	int tm = team;

	ImGui::TextColored(ImVec4(0.3f, 0.2f, 0.4f, 1.f), name.c_str());
	pos.ImGuiDraw("Position");
	dir.ImGuiDraw("Direction");
	ImGui::DragInt("Address",    &address, 1.f, 0, 0, "%#010x");
	ImGui::DragInt("NetworkId",  &networkId, 1.f, 0, 0, "%#010x");
	ImGui::DragInt("Index",      &idx);
	ImGui::DragInt("Team",       &tm);
	ImGui::DragFloat("LastSeen", &lastSeen);
	ImGui::DragFloat("FirstSeen", &firstSeen);
	ImGui::Checkbox("Visible",   &isVisible);
}
