#include "GameChampion.h"
#include "GameData.h"

GameChampion::GameChampion(std::string name)
	:GameUnit(name)
{
}

void GameChampion::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);

	/// Read spells
	int spellBook = addr + Offsets::ObjSpellBook;
	for (int i = 0; i < 6; ++i) {
		spells[i].ReadFromBaseAddress(ReadInt(spellBook + i * sizeof(int)));
	}

	/// Read items
	int itemList = ReadInt(addr + Offsets::ObjItemList);
	for (int i = 0; i < 6; ++i) {
		items[i] = nullptr;

		int item = ReadInt(itemList + i * 0x10 + Offsets::ItemListItem);
		if (item == 0)
			continue;

		int itemInfo = ReadInt(item + Offsets::ItemInfo);
		if (itemInfo == 0)
			continue;
		
		int id = ReadInt(itemInfo + Offsets::ItemInfoId);
		items[i] = GameData::GetItem(id);
	}
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
