#pragma once
#include <sstream>
#include <fstream>
#include <mutex>
#include <list>

// Thread safe
class Logger {

public:
	                               Logger(std::shared_ptr<std::iostream> stream);
	void                           Log(const char* str, ...);
	void                           GetLines(std::list<std::string>& lines);
private:
	std::shared_ptr<std::iostream> stream;
	std::mutex                     streamMutex;

public:
	static void   LogAll(const char* str, ...);
	static Logger File;
	static Logger Console;
};