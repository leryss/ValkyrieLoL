#pragma once
#include "GameState.h"
#include "Benchmark.h"

#include <queue>
#include <set>
#include <map>

class GameReader {

public:
	GameState& GetNextState();
	Benchmark& GetBenchmarks();

private:
	void        ReadObjectTree();
	int         ReadTreeNodes(std::queue<int>& nodesToVisit, int node);
	void        ReadGameObject(int address);

	void        AddToCache(GameObject* obj);
	GameObject* CreateObject(int addr);
	std::string PeekObjectName(int addr);

private:
	Benchmark                readBenchmarks;
	static std::string       BenchmarkReadTreeName;

	GameState                state;
	int                      baseAddr;
	std::set<int>            blacklistedObjects;
	std::set<int>            updatedObjects;
};