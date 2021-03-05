#include "GameReader.h"
#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"
#include "imgui/imgui.h"

#include "GameUnit.h"
#include "GameData.h"
#include "GameMissile.h"
#include "GameChampion.h"
#include "GameTurret.h"
#include "GameMinion.h"
#include "GameJungle.h"

#include <windows.h>

GameState* GameReader::GetNextState()
{
	benchmark.sehExceptions.value = 0;
	benchmark.cacheHits.value     = 0;
	benchmark.blacklistHits.value = 0;

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

		/// Read everything for local player, dont read buffs for enemies, dont read buffs, items & item actives for allies (performance reasons)
		for (auto& champ : state.champions) {
			if (champ == state.player) {
				champ->ReadSpells(GameChampion::NUM_SPELLS);
				champ->ReadItems();
				champ->ReadBuffs();
			}
			else if (champ->IsEnemyTo(*state.player)) {
				champ->ReadSpells(GameChampion::NUM_SPELLS);
				champ->ReadItems();
			}
			else {
				champ->ReadSpells(GameChampion::NUM_SPELLS - GameChampion::NUM_ITEMS);
			}
		}
	}
	
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
	int addr = ReadInt(baseAddr + Offsets::UnderMouseObject);

	if (!CantRead(addr)) {
		int netId = ReadInt(addr + Offsets::ObjNetworkID);
		auto find = state.objectCache.find(netId);
		state.hovered = (find == state.objectCache.end() ? nullptr : find->second);
	}
}

void GameReader::ReadObjectTree() {

	static const int      NUM_MAX_READS = 500; /// Used to prevent infinite loops due to race conditions
	static std::set<int>  ObjectPointers;

	ObjectPointers.clear();
	updatedObjects.clear();

	int objManager = ReadInt(baseAddr + Offsets::ObjectManager);
	int treeRoot   = ReadInt(objManager + Offsets::ObjectMapRoot);

	std::queue<int> nodesToVisit;
	std::set<int> visitedNodes;
	nodesToVisit.push(treeRoot);
	visitedNodes.insert(NULL);
	
	benchmark.readTree.Start();

	/// Read object addresses from tree
	int numObj = 0, reads = 0;
	int node;
	while (reads < NUM_MAX_READS && nodesToVisit.size() > 0) {
		node = nodesToVisit.front();
		nodesToVisit.pop();
		if (visitedNodes.find(node) != visitedNodes.end())
			continue;

		reads++;
		visitedNodes.insert(node);

		int addr = ReadTreeNodes(nodesToVisit, node);
		if (addr != NULL)
			ObjectPointers.insert(addr);

	}

	benchmark.readsPerformed.value = reads;
	benchmark.readTree.End();
	benchmark.readObjects.Start();
	benchmark.numObjPointers.value = ObjectPointers.size();

	/// Read objects using addresses read previously
	for (int ptr : ObjectPointers) {
		ReadGameObject(ptr);
	}

	/// Dispose of objects that were destroyed ingame
	auto it = state.objectCache.begin();
	while (it != state.objectCache.end()) {
		if (updatedObjects.find(it->first) == updatedObjects.end())
			it = state.objectCache.erase(it);
		it++;
	}
	
	benchmark.readObjects.End();
}

int GameReader::ReadTreeNodes(std::queue<int>& nodesToVisit, int node)
{
	static const int OBJ_NET_ID_START = 0x40000000;
	static const int OBJ_NET_ID_END = OBJ_NET_ID_START + 0x100000;

	__try {
		int childOne = ReadInt(node);
		int childTwo = ReadInt(node + sizeof(int));
		int childThree = ReadInt(node + 2 * sizeof(int));

		nodesToVisit.push(childOne);
		nodesToVisit.push(childTwo);
		nodesToVisit.push(childThree);

		int netId = ReadInt(node + Offsets::ObjectMapNodeNetId);

		/// Check if valid object by checking if it has a valid network id
		if (netId > OBJ_NET_ID_START && netId < OBJ_NET_ID_END) {
			return ReadInt(node + Offsets::ObjectMapNodeObject);
		}
	} 
	__except (EXCEPTION_EXECUTE_HANDLER) {
		benchmark.sehExceptions.value += 1;
	}
	return NULL;
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
	int missileName = ReadInt(addr + Offsets::ObjMissileName);
	name = Memory::ReadString(missileName);

	if (!name.empty()) {
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
	ImGui::Text("Game Reader Benchmarks");
	ImGui::DragFloat(readTree.name,     &readTree.avgMs);
	ImGui::DragFloat(readObjects.name,  &readObjects.avgMs);
	ImGui::Separator();
	ImGui::DragInt(sehExceptions.name,  &sehExceptions.value);
	ImGui::DragInt(readsPerformed.name, &readsPerformed.value);
	ImGui::DragInt(numObjPointers.name, &numObjPointers.value);
	ImGui::DragInt(cacheHits.name,      &cacheHits.value);
	ImGui::DragInt(blacklistHits.name,  &blacklistHits.value);
}
