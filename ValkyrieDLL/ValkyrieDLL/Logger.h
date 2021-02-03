#pragma once
#include <sstream>
#include <fstream>
#include <mutex>
#include <list>

// Thread safe
class Logger {

private:
	static std::shared_ptr<std::fstream>      FileStream;
	static std::shared_ptr<std::stringstream> ConsoleStream;

	static std::mutex                         FileMutex;
	static std::mutex                         ConsoleMutex;

public:
	static void   InitLoggers(const char* pathFileLog);

	static void   LogAll(const char* str, ...);
	static void   Console(const char* str, ...);
	static void   File(const char* str, ...);
	static void   GetConsoleLines(std::list<std::string>& lines);
};