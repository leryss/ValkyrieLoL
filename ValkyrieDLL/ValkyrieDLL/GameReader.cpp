#include "GameReader.h"
#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"
#include "imgui/imgui.h"

#include <windows.h>

GameState& GameReader::GetNextState()
{
	benchmark.sehExceptions.value = 0;
	benchmark.cacheHits.value     = 0;
	benchmark.blacklistHits.value = 0;

	baseAddr = (int)GetModuleHandle(NULL);
	memcpy(&state.time, (void*)(baseAddr + Offsets::GameTime), sizeof(float));
	
	if (state.time > 1.f) {
		state.renderer.ReadFromBaseAddress(baseAddr);
		state.hud.ReadFromBaseAddress(baseAddr);

		ReadObjectTree();
	}
	return state;
}

BenchmarkGameReader& GameReader::GetBenchmarks()
{
	return benchmark;
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
	__except (1) {
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
	PeekObjectName(addr, name);
	if (name.empty())
		return nullptr;

	GameObject* obj = new GameObject(name);
	return obj;
}

void GameReader::PeekObjectName(int addr, std::string& name) {

	int nameAddr = ReadInt(addr + Offsets::ObjName);
	if (!CantRead((void*)nameAddr, 1))
		name = Memory::ReadString(nameAddr);
	
	if (name.empty()) {
		int missileSpell = ReadInt(addr + Offsets::MissileSpellInfo);
		if (CantRead(missileSpell))
			return;

		int missileSpellData = ReadInt(missileSpell + Offsets::SpellInfoSpellData);
		if (CantRead(missileSpellData))
			return;

		nameAddr = ReadInt(missileSpellData + Offsets::SpellDataSpellName);
		if (CantRead(nameAddr))
			return;
		name = Memory::ReadString(nameAddr);
	}
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

		updatedObjects.insert(obj->networkId);
	}
	__except (1) {
		benchmark.sehExceptions.value += 1;
		if (obj != nullptr)
			delete obj;
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
