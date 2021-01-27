#pragma once
#include <map>
#include <chrono>

using namespace std::chrono;

class BenchmarkInfo {
public:
	float                             lastMs    = 0.f;
	float                             avgMs     = 0.f;
	high_resolution_clock::time_point timeBegin;
};

class Benchmark {

public:
	void  StartFor(std::string& str);
	void  EndFor(std::string& str);

public:
	std::map<std::string, BenchmarkInfo> benchmarks;
};