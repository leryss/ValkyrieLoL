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
	spells[0].castKey  = HKey::Q;
	spells[1].castKey  = HKey::W;
	spells[2].castKey  = HKey::E;
	spells[3].castKey  = HKey::R;
					   
	spells[4].castKey  = HKey::D;
	spells[5].castKey  = HKey::F;
					   
	spells[6].castKey  = HKey::N_1;
	spells[7].castKey  = HKey::N_2;
	spells[8].castKey  = HKey::N_3;
	spells[9].castKey  = HKey::N_4;
	spells[10].castKey = HKey::N_5;
	spells[11].castKey = HKey::N_6;
}

void GameChampion::ReadBuffs(int addr)
{
	buffs.clear();

	int buffManager = addr + Offsets::ObjBuffManager;
	int buffArray = ReadInt(buffManager + Offsets::BuffManagerEntriesArray);

	if (CantRead(buffArray))
		return;

	int i = 0;
	while (true) {
		int buffEntry = ReadInt(buffArray + i * 0x8);
		i++;

		if (CantRead(buffEntry))
			break;

		auto buff = new GameBuff();
		buff->ReadFromBaseAddress(buffEntry);

		if (buff->name.empty())
			continue;

		buffs[buff->name] = std::shared_ptr<GameBuff>(buff);
	}
}

void GameChampion::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);

	/// Read spells
	int spellBook  = addr + Offsets::ObjSpellBook;
	int spellSlots = spellBook + Offsets::SpellBookSpellSlots;
	for (int i = 0; i < NUM_SPELLS; ++i) {
		spells[i].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
	}

	/// Read items
	int itemList = ReadInt(addr + Offsets::ObjItemList);
	if (CantRead(itemList))
		return;

	for (int i = 0; i < NUM_ITEMS; ++i) {
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
	recalling = (ReadInt(addr + Offsets::ObjRecallState) == 6);

	ReadBuffs(addr);
}

void GameChampion::ImGuiDraw()
{
	GameUnit::ImGuiDraw();
	ImGui::Separator();

	if (ImGui::TreeNode("Buffs")) {
		for (auto& pair : buffs) {
			if (ImGui::TreeNode(pair.first.c_str())) {
				pair.second->ImGuiDraw();
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Items")) {
		for (int i = 0; i < NUM_ITEMS; ++i) {
			if (items[i] == nullptr)
				continue;

			if (ImGui::TreeNode(Strings::Format("%d", items[i]->id).c_str())) {
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	ImGui::Text("Spells");
	for (int i = 0; i < NUM_SPELLS; ++i) {
		if (i == 4 || i == 6)
			ImGui::Separator();

		if (!spells[i].name.empty() && ImGui::TreeNode(spells[i].name.c_str())) {

			spells[i].ImGuiDraw();
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

list GameChampion::BuffsToPy()
{
	list l = list();
	for (auto& pair : buffs) {
		l.append(ptr(pair.second.get()));
	}

	return l;
}

object GameChampion::SpellsToPy()
{
	list l;
	for (int i = 0; i < NUM_SPELLS; ++i)
		l.append(boost::ref(spells[i]));

	return l;
}

object GameChampion::ItemsToPy()
{
	list l;
	for (int i = 0; i < NUM_ITEMS; ++i)
		l.append(ptr(items[i]));
	
	return l;
}

bool GameChampion::HasBuff(const char * buff)
{
	return buffs.find(std::string(buff)) != buffs.end();
}
