#include "GameReader.h"



#include "imgui/imgui.h"

#include "GameUnit.h"
#include "GameData.h"
#include "GameMissile.h"
#include "GameChampion.h"
#include "GameTurret.h"
#include "GameMinion.h"
#include "GameJungle.h"

#include "Debug.h"

#include <windows.h>

GameState* GameReader::GetNextState()
{
	DBG_INFO("GameReader::GetNextState")
	benchmark.sehExceptions.value = 0;
	benchmark.cacheHits.value     = 0;
	benchmark.blacklistHits.value = 0;

	benchmark.readState.Start();

	baseAddr = (int)GetModuleHandle(NULL);
	state.time = ReadFloat(baseAddr + Offsets::GameTime);

	/// Ghetto way of checking if game has started
	if (state.time > 1.f) {
		state.hovered = nullptr;
		state.gameStarted = true;

		state.renderer.ReadFromBaseAddress(baseAddr);
		state.hud.ReadFromBaseAddress(baseAddr);

		ReadObjectTree();
		SieveObjects();
		ReadLocalChampion();
		ReadHoveredObject();
		ReadFocusedObject();
		
		/// Read everything for local player, dont read buffs for enemies, dont read buffs, items & item actives for allies (performance reasons)
		for (auto& champ : state.champions) {
			if (champ == state.player) {
				champ->ReadSpells(GameChampion::NUM_SPELLS);
				champ->ReadItems();
			}
			else if (champ->IsEnemyTo(*state.player)) {
				champ->ReadSpells(GameChampion::NUM_SPELLS);
				if(!champ->IsClone())
					champ->ReadItems();
			}
			else {
				champ->ReadSpells(GameChampion::NUM_SPELLS - GameChampion::NUM_ITEMS);
			}
		}
		
		if(state.turrets.size() > 0)
			state.map.type = (MapType)*(short*)state.turrets[0]->name.c_str();
	}

	benchmark.readState.End();

	return &state;
}

GameState * GameReader::GetCurrentState()
{
	return &state;
}

BenchmarkGameReader& GameReader::GetBenchmarks()
{
	return benchmark;
}

void GameReader::ReadLocalChampion()
{
	DBG_INFO("GameReader::ReadLocalChampion")
	int addr = ReadInt(baseAddr + Offsets::LocalPlayer);

	/// In Spectator/Replays the local champion is nullptr so we just choose the first champion
	if (CantRead(addr) && state.champions.size() > 0)
		state.player = state.champions[0];
	else {
		int netId = ReadInt(addr + Offsets::ObjNetworkID);
		auto find = state.objectCache.find(netId);
		state.player = std::dynamic_pointer_cast<GameChampion>(find == state.objectCache.end() ? nullptr : find->second);
	}
}

void GameReader::ReadHoveredObject()
{
	DBG_INFO("GameReader::ReadHoveredObject")

	state.hovered = nullptr;
	int mouseInfo = ReadInt(baseAddr + Offsets::MouseInfo);
	int hoveredObject = ReadInt(mouseInfo + Offsets::MouseInfoHoveredObject);

	if (hoveredObject != 0) {
		int netId = ReadInt(hoveredObject + Offsets::ObjNetworkID);
		auto find = state.objectCache.find(netId);
		if(find != state.objectCache.end())
			state.hovered = find->second;
	}
}

void GameReader::ReadFocusedObject()
{
	DBG_INFO("GameReader::ReadFocusedObject")
	if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) {
		state.focused = state.hovered;
	}
}

void GameReader::ReadObjectTree()
{
	static const int OBJ_NET_ID_START = 0x40000000;
	static const int OBJ_NET_ID_END = OBJ_NET_ID_START + 0x100000;

	int objManager = ReadInt(baseAddr + Offsets::ObjectManager);
	std::map<int, int>* objMap = (std::map<int, int>*)(objManager + Offsets::ObjectMapRoot);

	benchmark.readObjects.Start();
	updatedObjects.clear();
	for (auto pair : *objMap) {
		if (pair.first > OBJ_NET_ID_START && pair.first < OBJ_NET_ID_END) {
			ReadGameObject(pair.second);
		}
	}

	auto it = state.objectCache.begin();
	while (it != state.objectCache.end()) {
		if (updatedObjects.find(it->first) == updatedObjects.end())
			it = state.objectCache.erase(it);
		it++;
	}
	benchmark.readObjects.End();
}


void GameReader::AddToCache(GameObject* obj) {
	state.objectCache[obj->networkId] = std::shared_ptr<GameObject>(obj);
}

GameObject* GameReader::CreateObject(int addr)
{
	std::string name;

	/// Try to read unit name
	int nameAddr = ReadInt(addr + Offsets::ObjName);
	if (!CantRead((void*)nameAddr))
		name = Memory::ReadString(nameAddr);

	if (!name.empty()) {
		DBG_INFO("GameReader::CreateObject %s", name.c_str())
		name = Strings::ToLower(name);
		UnitInfo* info = GameData::GetUnit(name);
		if (info == nullptr) 
			return nullptr;

		if (info->HasTag(Unit_Champion))
			return new GameChampion(name);
		else if (info->HasTag(Unit_Minion_Lane))
			return new GameMinion(name);
		else if (info->HasTag(Unit_Structure_Turret))
			return new GameTurret(name);
		else if (info->HasTag(Unit_Monster))
			return new GameJungle(name);
		else
			return new GameUnit(name);
	}
		

	/// Try to read missile name
	int spellInfo = ReadInt(addr + Offsets::ObjMissileSpellCast + Offsets::SpellCastSpellInfo);
	if (CantRead(spellInfo))
		return nullptr;
	int spellData = ReadInt(spellInfo + Offsets::SpellInfoSpellData);
	if (CantRead(spellData))
		return nullptr;
	nameAddr = ReadInt(spellData + Offsets::SpellDataSpellName);
	if (CantRead(nameAddr))
		return nullptr;

	name = Memory::ReadString(nameAddr);
	if (!name.empty()) {
		DBG_INFO("GameReader::CreateObject %s", name.c_str())
		name = Strings::ToLower(name);
		if (GameData::GetSpell(name) == nullptr) 
			return nullptr;

		return new GameMissile(name);
	}
		
	return nullptr;
}

void GameReader::ReadGameObject(int address)
{
	GameObject* obj = nullptr;
	int netId = 0;

	__try {
		int netId = ReadInt(address + Offsets::ObjNetworkID);
		if (blacklistedObjects.find(netId) != blacklistedObjects.end()) {
			benchmark.blacklistHits.value += 1;
			return;
		}

		auto& objectCache = state.objectCache;
		auto find = objectCache.find(netId);
		if (find == objectCache.end()) {
			obj = CreateObject(address);
			if (obj == nullptr) {
				blacklistedObjects.insert(netId);
				return;
			}

			obj->ReadFromBaseAddress(address);
			obj->firstSeen = state.time;
			AddToCache(obj);
		}
		else {
			benchmark.cacheHits.value += 1;
			obj = find->second.get();
			obj->ReadFromBaseAddress(address);
		}

		if (obj->isVisible)
			obj->lastSeen = state.time;

		updatedObjects.insert(obj->networkId);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		benchmark.sehExceptions.value += 1;
		if (obj != nullptr)
			delete obj;
	}
}

void GameReader::SieveObjects()
{
	state.missiles.clear();
	state.champions.clear();
	state.minions.clear();
	state.turrets.clear();
	state.jungle.clear();
	state.others.clear();

	for (auto& pair : state.objectCache) {
		switch (pair.second->type) {
		
		case OBJ_CHAMPION:
			state.champions.push_back(std::dynamic_pointer_cast<GameChampion>(pair.second));
			break;
		case OBJ_MINION:
			state.minions.push_back(std::dynamic_pointer_cast<GameMinion>(pair.second));
			break;
		case OBJ_TURRET:
			state.turrets.push_back(std::dynamic_pointer_cast<GameTurret>(pair.second));
			break;
		case OBJ_MISSILE:
			state.missiles.push_back(std::dynamic_pointer_cast<GameMissile>(pair.second));
			break;
		case OBJ_JUNGLE:
			state.jungle.push_back(std::dynamic_pointer_cast<GameJungle>(pair.second));
			break;
		case OBJ_UNKNOWN:
		default:
			state.others.push_back(std::dynamic_pointer_cast<GameUnit>(pair.second));
		}
	}
}

void BenchmarkGameReader::ImGuiDraw()
{
	ImGui::DragFloat(readTree.name,     &readTree.avgMs);
	ImGui::DragFloat(readObjects.name,  &readObjects.avgMs);
	ImGui::DragFloat(readState.name,    &readState.avgMs);
	ImGui::Separator();
	ImGui::DragInt(sehExceptions.name,  &sehExceptions.value);
	ImGui::DragInt(cacheHits.name,      &cacheHits.value);
	ImGui::DragInt(blacklistHits.name,  &blacklistHits.value);
}
