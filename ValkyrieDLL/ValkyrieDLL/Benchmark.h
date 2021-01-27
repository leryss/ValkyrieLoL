#pragma once
#include <map>
#include <chrono>

using namespace std::chrono;

class BenchmarkTiming {
public:
	BenchmarkTiming(const char* name);

	void Start();
	void End();

	const char*                       name;
	float                             lastMs    = 0.f;
	float                             avgMs     = 0.f;
	high_resolution_clock::time_point timeBegin;
};

template<class T>
class BenchmarkValue {
public:
	BenchmarkValue(const char* name) {
		this->name = name;
	}

	const char* name;
	T           value;
};
