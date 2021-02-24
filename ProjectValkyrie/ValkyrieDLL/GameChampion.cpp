#include "GameChampion.h"
#include "GameData.h"
#include "Valkyrie.h"

GameChampion::GameChampion()
{
}

GameChampion::GameChampion(std::string name)
	:GameUnit(name)
{
	type = OBJ_CHAMPION;
	spells[0].castKey = HKey::Q;
	spells[1].castKey = HKey::W;
	spells[2].castKey = HKey::E;
	spells[3].castKey = HKey::R;

	spells[4].castKey = HKey::D;
	spells[5].castKey = HKey::F;
}

void GameChampion::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);

	/// Read spells
	int spellBook  = addr + Offsets::ObjSpellBook;
	int spellSlots = spellBook + Offsets::SpellBookSpellSlots;
	for (int i = 0; i < 6; ++i) {
		spells[i].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
	}

	/// Read items
	int itemList = ReadInt(addr + Offsets::ObjItemList);
	if (CantRead(itemList))
		return;

	for (int i = 0; i < 6; ++i) {
		items[i] = nullptr;

		int item = ReadInt(itemList + i * 0x10 + Offsets::ItemListItem);
		if (CantRead(item))
			continue;

		int itemInfo = ReadInt(item + Offsets::ItemInfo);
		if (CantRead(itemInfo))
			continue;
		
		int id = ReadInt(itemInfo + Offsets::ItemInfoId);
		items[i] = GameData::GetItem(id);
	}

	/// Check recalling
	recalling = ReadInt(addr + Offsets::ObjRecallState);
}

void GameChampion::ImGuiDraw()
{
	GameUnit::ImGuiDraw();
	ImGui::Separator();

	ImGui::Text("Spells");
	for (int i = 0; i < 6; ++i) {
		if (ImGui::TreeNode(spells[i].name.c_str())) {
			
			spells[i].ImGuiDraw();
			ImGui::TreePop();
		}
	}

	ImGui::Text("Items");
	for (int i = 0; i < 6; ++i) {
		if (items[i] == nullptr)
			continue;

		if (ImGui::TreeNode(Strings::Format("%d", items[i]->id).c_str())) {
			ImGui::TreePop();
		}
	}
}

Vector2 GameChampion::GetHpBarPosition()
{
	Vector3 p = pos.clone();
	p.y += staticData->healthBarHeight;

	Vector2 w2s = Valkyrie::CurrentGameState->renderer.WorldToScreen(p);
	w2s.y -= (Valkyrie::CurrentGameState->renderer.height * 0.00083333335f * staticData->healthBarHeight);
	w2s.x -= 70.f;
	return w2s;
}

object GameChampion::SpellsToPy()
{
	list l;
	for (int i = 0; i < 6; ++i)
		l.append(boost::ref(spells[i]));

	return l;
}

object GameChampion::ItemsToPy()
{
	list l;
	for (int i = 0; i < 6; ++i)
		l.append(ptr(items[i]));
	
	return l;
}