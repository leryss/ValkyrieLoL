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

void GameChampion::ReadBuffs(int addr)
{
	buffs.clear();

	int buffManager = addr + 0x2178;
	int buffArray = ReadInt(buffManager + 0x10);

	if (CantRead(buffArray))
		return;

	float gameTime = ReadFloat((int)GetModuleHandle(NULL) + Offsets::GameTime);

	int i = 0;
	while (true) {
		int buffEntry = ReadInt(buffArray + i * 0x8);
		i++;

		if (CantRead(buffEntry))
			break;

		float buffEndTime = ReadFloat(buffEntry + 0x10);
		if (buffEndTime < gameTime)
			continue;

		int buff = ReadInt(buffEntry + 0x8);
		if (CantRead(buff))
			continue;

		buffs.insert(Memory::ReadString(buff + 0x8, 100));
	}
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
	recalling = (ReadInt(addr + Offsets::ObjRecallState) == 6);

	ReadBuffs(addr);
}

void GameChampion::ImGuiDraw()
{
	GameUnit::ImGuiDraw();
	ImGui::Separator();

	if (ImGui::TreeNode("Buffs")) {
		for (auto& buff : buffs)
			ImGui::TextColored(Color::YELLOW, buff.c_str());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Items")) {
		for (int i = 0; i < 6; ++i) {
			if (items[i] == nullptr)
				continue;

			if (ImGui::TreeNode(Strings::Format("%d", items[i]->id).c_str())) {
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	ImGui::Text("Spells");
	for (int i = 0; i < 6; ++i) {
		if (ImGui::TreeNode(spells[i].name.c_str())) {

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

bool GameChampion::HasBuff(const char * buff)
{
	return buffs.find(std::string(buff)) != buffs.end();
}
