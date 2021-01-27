#include "Benchmark.h"

void Benchmark::StartFor(std::string & str)
{
	auto find = benchmarks.find(str);
	if (find == benchmarks.end())
		benchmarks[str] = BenchmarkInfo();

	BenchmarkInfo& info = benchmarks[str];
	info.timeBegin = high_resolution_clock::now();
}

void Benchmark::EndFor(std::string & str)
{
	BenchmarkInfo& info = benchmarks[str];
	duration<float, std::milli> duration = (high_resolution_clock::now() - info.timeBegin);

	info.lastMs = duration.count();
	info.avgMs = (info.avgMs + info.lastMs) / 2.f;
}
