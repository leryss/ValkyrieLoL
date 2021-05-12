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

	for (int i = 0; i < NUM_ABILITIES; ++i)
		pySpells.append(boost::ref(abilities[i]));

	for (int i = 0; i < NUM_SUMMONERS; ++i)
		pySpells.append(boost::ref(summoners[i]));

	for (int i = 0; i < NUM_ITEMS; ++i) {
		pySpells.append(boost::ref(itemSpells[i]));
		pyItems.append(boost::ref(items[i]));
	}

	abilities[0].castKey = GameKeybind::CastSpellQ;
	abilities[1].castKey = GameKeybind::CastSpellW;
	abilities[2].castKey = GameKeybind::CastSpellE;
	abilities[3].castKey = GameKeybind::CastSpellR;

	summoners[0].castKey = GameKeybind::CastSpellD;
	summoners[1].castKey = GameKeybind::CastSpellF;

	itemSpells[0].castKey = GameKeybind::UseItem1;
	itemSpells[1].castKey = GameKeybind::UseItem2;
	itemSpells[2].castKey = GameKeybind::UseItem3;
	itemSpells[3].castKey = GameKeybind::UseItem4;
	itemSpells[4].castKey = GameKeybind::UseItem5;
	itemSpells[5].castKey = GameKeybind::UseItem6;
	itemSpells[6].castKey = GameKeybind::UseItemTrinket;
}

void GameChampion::ReadSpells(int numToRead)
{
	DBG_INFO("Reading %d spells for %s", numToRead, name.c_str());
	int spellBook = address + Offsets::ObjSpellBook;
	int spellSlots = spellBook + Offsets::SpellBookSpellSlots;
	int castableMask = ReadInt(spellBook + Offsets::SpellBookCastableMask);

	int end = min(numToRead, NUM_ABILITIES);
	for (int i = 0; i < end; ++i) {
		abilities[i].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
		abilities[i].castableBit = castableMask & (1 << i);
	}

	end = min(numToRead, NUM_ABILITIES + NUM_SUMMONERS);
	for (int i = NUM_ABILITIES, j = 0; i < end; ++i, ++j) {
		summoners[j].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
		summoners[j].castableBit = castableMask & (1 << i);
	}

	end = min(numToRead, NUM_ABILITIES + NUM_SUMMONERS + NUM_ITEMS);
	for (int i = (NUM_ABILITIES + NUM_SUMMONERS), j = 0; i < end; ++i, ++j) {
		itemSpells[j].ReadFromBaseAddress(ReadInt(spellSlots + i * sizeof(int)));
		itemSpells[j].castableBit = castableMask & (1 << i);
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
			for (size_t j = 0; j < NUM_ITEMS; ++j) {
				if (itemSpells[j].lvl > 0 && itemSpells[j].name == activeName) {
					items[i].active = &itemSpells[j];
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
	if (castingSpell.staticData != nullptr)
		channeling = isCasting && castingSpell.staticData->HasFlag(ChannelSkill);
	ReadAiManager();
}

void GameChampion::ImGuiDraw()
{
	GameUnit::ImGuiDraw();
	ImGui::Separator();

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
	for (int i = 0; i < NUM_ABILITIES; ++i) {
		if (!abilities[i].name.empty() && ImGui::TreeNode(abilities[i].name.c_str())) {
			abilities[i].ImGuiDraw();
			ImGui::TreePop();
		}
	}
	ImGui::Separator();
	for (int i = 0; i < NUM_SUMMONERS; ++i) {
		if (!summoners[i].name.empty() && ImGui::TreeNode(summoners[i].name.c_str())) {
			summoners[i].ImGuiDraw();
			ImGui::TreePop();
		}
	}
	ImGui::Separator();
	for (int i = 0; i < NUM_ITEMS; ++i) {
		if (!itemSpells[i].name.empty() && ImGui::TreeNode(itemSpells[i].name.c_str())) {
			itemSpells[i].ImGuiDraw();
			ImGui::TreePop();
		}
	}

	ImGui::Checkbox("Channeling", &channeling);
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

object GameChampion::SpellsToPy()
{
	return pySpells;
}

object GameChampion::ItemsToPy()
{
	return pyItems;
}

object GameChampion::GetQ()
{
	return pySpells[0];
}

object GameChampion::GetW()
{
	return pySpells[1];
}

object GameChampion::GetE()
{
	return pySpells[2];
}

object GameChampion::GetR()
{
	return pySpells[3];
}

object GameChampion::GetSummoner(SummonerType type)
{
	if (summoners[0].type == type)
		return pySpells[NUM_ABILITIES];
	if (summoners[1].type == type)
		return pySpells[NUM_ABILITIES + 1];

	return object();
}

object GameChampion::GetItem(int id)
{
	for (int i = 0; i < NUM_ITEMS; ++i) {
		if (items[i].item != nullptr && items[i].item->id == id)
			return object(boost::ref(items[i]));
	}

	return object();
}

bool GameChampion::HasItem(int id)
{
	for (int i = 0; i < NUM_ITEMS; ++i) {
		if (items[i].item != nullptr && items[i].item->id == id)
			return true;
	}

	return false;
}

bool GameChampion::CanCast(const GameSpell * spell)
{
	return mana >= spell->mana && spell->GetRemainingCooldown() == 0.0f && spell->castableBit;
}

bool GameChampion::IsClone() const
{
	return summoners[0].name == summoners[1].name;
}
