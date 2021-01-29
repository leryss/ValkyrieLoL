#pragma once
#include "GameState.h"
#include "Benchmark.h"

#include <queue>
#include <set>
#include <map>

class BenchmarkGameReader {

public:
	BenchmarkTiming readTree    = BenchmarkTiming("Read Obj Tree");
	BenchmarkTiming readObjects = BenchmarkTiming("Read Game Objects");
	
	BenchmarkValue<int> sehExceptions  = BenchmarkValue<int>("SEH Exceptions");
	BenchmarkValue<int> readsPerformed = BenchmarkValue<int>("Obj Tree Node Reads");
	
	BenchmarkValue<int> cacheHits      = BenchmarkValue<int>("Cache Hits");
	BenchmarkValue<int> blacklistHits  = BenchmarkValue<int>("Blacklist Hits");
	BenchmarkValue<int> numObjPointers = BenchmarkValue<int>("Obj Pointers Read");

	void ImGuiDraw();
};

class GameReader {

public:
	GameState&           GetNextState();
	BenchmarkGameReader& GetBenchmarks();

public:

private:
	void        ReadLocalChampion();
	void        ReadHoveredObject();
	void        ReadObjectTree();
	int         ReadTreeNodes(std::queue<int>& nodesToVisit, int node);
	void        ReadGameObject(int address);
	void        SieveObjects();

	void        AddToCache(GameObject* obj);
	GameObject* CreateObject(int addr);

private:

	BenchmarkGameReader      benchmark;

	GameState                state;
	int                      baseAddr;
	std::set<int>            updatedObjects;
	std::set<int>            blacklistedObjects;
};