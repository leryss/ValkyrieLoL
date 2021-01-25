#pragma once
#include <sstream>
#include <fstream>

class Logger {

public:
	                               Logger(std::shared_ptr<std::iostream> stream);
	void                           Log(const char* str, ...);
	std::shared_ptr<std::iostream> GetStream();
private:
	std::shared_ptr<std::iostream> stream;

public:
	static Logger FileLogger;
	static Logger ConsoleLogger;
};