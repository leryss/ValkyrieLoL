#include "Benchmark.h"

BenchmarkTiming::BenchmarkTiming(const char * name)
{
	this->name = name;
}

void BenchmarkTiming::Start()
{
	timeBegin = high_resolution_clock::now();
}

void BenchmarkTiming::End()
{
	duration<float, std::milli> dur = high_resolution_clock::now() - timeBegin;
	lastMs = dur.count();
	avgMs  = (avgMs + lastMs) / 2.f;
}
