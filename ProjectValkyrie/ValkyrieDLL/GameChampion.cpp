#include "GameChampion.h"
#include "GameData.h"
#include "Valkyrie.h"
#include "GameKeybind.h"

GameChampion::GameChampion()
{
}

GameChampion::GameChampion(std::string name)
	:GameUnit(name)
{
	type = OBJ_CHAMPION;
	spells[0].castKey  = GameKeybind::CastSpellQ;
	spells[1].castKey  = GameKeybind::CastSpellW;
	spells[2].castKey  = GameKeybind::CastSpellE;
	spells[3].castKey  = GameKeybind::CastSpellR;
					   
	spells[4].castKey  = GameKeybind::CastSpellD;
	spells[5].castKey  = GameKeybind::CastSpellF;
					   
	spells[6].castKey  = GameKeybind::UseItem1;
	spells[7].castKey  = GameKeybind::UseItem2;
	spells[8].castKey  = GameKeybind::UseItem3;
	spells[9].castKey  = GameKeybind::UseItem4;
	spells[10].castKey = GameKeybind::UseItem5;
	spells[11].castKey = GameKeybind::UseItem6;
	spells[12].castKey = GameKeybind::UseItemTrinket;
}

void GameChampion::ReadSpells(int numToRead)
{
	DBG_INFO("Reading %d spells for %s", numToRead, name.c_str());
	int spellBook = address + Offsets::ObjSpellBook;
	int spellSlots = spellBook + Offsets::SpellBookSpellSlots;
	int castableMask = ReadInt(spellBook + Offsets::SpellBookCastableMask);

	for (int i = 0; i < numToRead; ++i) {
		spells[i].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
		spells[i].castableBit = castableMask & (1 << i);
	}
}

void GameChampion::ReadBuffs()
{
	DBG_INFO("Reading buffs for %s", name.c_str());
	buffs.clear();

	int buffManager = address + Offsets::ObjBuffManager;
	int buffArray = ReadInt(buffManager + Offsets::BuffManagerEntriesArray);

	if (CantRead(buffArray)) {
		return;
	}

	int i = 0;
	while (true) {
		int buffEntry = ReadInt(buffArray + i * 0x8);
		i++;

		if (CantRead(buffEntry))
			break;

		auto buff = std::shared_ptr<GameBuff>(new GameBuff());
		buff->ReadFromBaseAddress(buffEntry);

		if (buff->name.empty())
			continue;

		buffs[buff->name] = buff;
	}
}

void GameChampion::ReadItems()
{
	DBG_INFO("Reading items for %s", name.c_str());
	int itemList = address + Offsets::ObjItemList;

	for (int i = 0; i < NUM_ITEMS; ++i) {
		items[i].item = nullptr;
		items[i].active = nullptr;

		int itemSlot = ReadInt(itemList + sizeof(int)*i);
		int item = ReadInt(itemSlot + Offsets::ItemListItem);
		if (CantRead(item))
			continue;

		int itemInfo = ReadInt(item + Offsets::ItemInfo);
		if (CantRead(itemInfo))
			continue;

		int id = ReadInt(itemInfo + Offsets::ItemInfoId);
		auto info = GameData::GetItem(id);
		if (info != nullptr) {
			/// Read active spell name
			int activeNameAddr = ReadInt(item + Offsets::ItemActiveName);
			if (CantRead(activeNameAddr))
				continue;
			std::string activeName = Memory::ReadString(activeNameAddr);
			activeName = Strings::ToLower(activeName);

			/// Find active spell in spell book
			for (size_t j = 6; j < NUM_SPELLS; ++j) {
				if (spells[j].lvl > 0 && spells[j].name == activeName) {
					items[i].active = &spells[j];
					break;
				}
			}

			items[i].item = info;
			items[i].charges = ReadShort(item + Offsets::ItemCharges);
		}
	}
}

void GameChampion::ReadFromBaseAddress(int addr)
{
	GameUnit::ReadFromBaseAddress(addr);

	/// Check recalling
	recalling = (ReadInt(addr + Offsets::ObjRecallState) == 6);
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
			if (items[i].item == nullptr) {
				ImGui::Text("Empty");
				continue;
			}

			if (ImGui::TreeNode(Strings::Format("%d", items[i].item->id).c_str())) {
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

Vector2 GameChampion::GetHpBarPosition() const
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
		l.append(boost::ref(items[i]));
	
	return l;
}

bool GameChampion::CanCast(const GameSpell * spell)
{
	return mana > spell->mana && spell->GetRemainingCooldown() == 0.0f && spell->castableBit;
}

bool GameChampion::HasBuff(const char * buff)
{
	return buffs.find(std::string(buff)) != buffs.end();
}

int GameChampion::BuffStackCount(const char * buff)
{
	std::string buffName = buff;
	for (auto& buff : buffs)
		if (buff.second->name == buffName)
			return buff.second->count;

	return 0;
}

bool GameChampion::IsClone() const
{
	return spells[4].name == spells[5].name;
}
