#include "GameReader.h"
#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"

#include <windows.h>


std::string GameReader::BenchmarkReadTreeName("ReadObjTree");

GameState& GameReader::GetNextState()
{
	baseAddr = (int)GetModuleHandle(NULL);
	memcpy(&state.time, (void*)(baseAddr + Offsets::GameTime), sizeof(float));
	
	if (state.time > 1.f) {
		state.renderer.ReadFromBaseAddress(baseAddr);
		state.hud.ReadFromBaseAddress(baseAddr);

		ReadObjectTree();
	}
	return state;
}

Benchmark & GameReader::GetBenchmarks()
{
	return readBenchmarks;
}

void GameReader::ReadObjectTree() {

	static const int      NUM_MAX_READS = 500; /// Used to prevent infinite loops due to race conditions
	static std::set<int>  ObjectPointers;

	readBenchmarks.StartFor(BenchmarkReadTreeName);

	ObjectPointers.clear();
	updatedObjects.clear();

	int objManager = ReadInt(baseAddr + Offsets::ObjectManager);
	int treeRoot   = ReadInt(objManager + Offsets::ObjectMapRoot);

	std::queue<int> nodesToVisit;
	std::set<int> visitedNodes;
	nodesToVisit.push(treeRoot);
	visitedNodes.insert(NULL);
	
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
	
	readBenchmarks.EndFor(BenchmarkReadTreeName);
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
	__except (1) {}
	return NULL;
}

void GameReader::AddToCache(GameObject* obj) {
	state.objectCache[obj->networkId] = std::shared_ptr<GameObject>(obj);
}

GameObject* GameReader::CreateObject(int addr)
{
	std::string name = PeekObjectName(addr);
	if (name.empty())
		return nullptr;

	
	GameObject* obj = new GameObject(name);
	return obj;
}

std::string GameReader::PeekObjectName(int addr) {
	
	std::string name = Memory::ReadString(ReadInt(addr + Offsets::ObjName));
	if (name.empty()) {
		int missileSpell     = ReadInt(addr + Offsets::MissileSpellInfo);
		int missileSpellData = ReadInt(missileSpell + Offsets::SpellInfoSpellData);
		name = Memory::ReadString(ReadInt(missileSpellData + Offsets::SpellDataSpellName));
	}

	return name;
}

void GameReader::ReadGameObject(int address)
{
	GameObject* obj = nullptr;
	__try {
		int netId = ReadInt(address + Offsets::ObjNetworkID);
		if (blacklistedObjects.find(netId) != blacklistedObjects.end())
			return;

		auto& objectCache = state.objectCache;
		auto find = objectCache.find(netId);
		if (find == objectCache.end()) {
			obj = CreateObject(address);

			/// If we can't create the object we blacklist it for performance
			if (obj == nullptr) {
				blacklistedObjects.insert(netId);
				return;
			}

			obj->ReadFromBaseAddress(address);
			AddToCache(obj);
		}
		else {
			obj = find->second.get();
			obj->ReadFromBaseAddress(address);
		}

		updatedObjects.insert(obj->networkId);
	}
	__except (1) {
		if (obj != nullptr)
			delete obj;
	}
}
