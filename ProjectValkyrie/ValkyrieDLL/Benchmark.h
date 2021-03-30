#pragma once
#include <map>
#include <chrono>

using namespace std::chrono;

class BenchmarkTiming {
public:
	BenchmarkTiming();
	BenchmarkTiming(const char* name);

	/// Starts the timer
	void Start();

	/// Ends the timer and updates all values (lastMs, avgMs etc)
	void End();

	const char*                       name;
	float                             lastMs    = 0.f;
	float                             avgMs     = 0.f;
	high_resolution_clock::time_point timeBegin;
};

template<class T>
class BenchmarkValue {
public:
	BenchmarkValue();
	BenchmarkValue(const char* name);

	const char* name;
	T           value;
};

template<class T>
inline BenchmarkValue<T>::BenchmarkValue()
{
	this->name = "Unnamed";
}

template<class T>
inline BenchmarkValue<T>::BenchmarkValue(const char * name) {
	this->name = name;
}