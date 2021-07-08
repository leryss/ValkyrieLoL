#pragma once
#include "GameState.h"


#include <queue>
#include <set>
#include <map>

class BenchmarkGameReader {

public:
	BenchmarkTiming readTree    = BenchmarkTiming("Read Obj Tree");
	BenchmarkTiming readObjects = BenchmarkTiming("Read Game Objects");
	BenchmarkTiming readState   = BenchmarkTiming("Read Game State");
	
	BenchmarkValue<int> sehExceptions  = BenchmarkValue<int>("SEH Exceptions");
	
	BenchmarkValue<int> cacheHits      = BenchmarkValue<int>("Cache Hits");
	BenchmarkValue<int> blacklistHits  = BenchmarkValue<int>("Blacklist Hits");

	void ImGuiDraw();
};

class GameReader {

public:
	GameState*           GetNextState();
	GameState*           GetCurrentState();
	BenchmarkGameReader& GetBenchmarks();

public:

private:
	void        ReadLocalChampion();
	void        ReadHoveredObject();
	void        ReadFocusedObject();
	void        ReadObjectTree();
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