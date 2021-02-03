#include "Logger.h"
#include <stdarg.h>
#include "Valkyrie.h"

std::shared_ptr<std::fstream>      Logger::FileStream    = nullptr;
std::shared_ptr<std::stringstream> Logger::ConsoleStream = nullptr;
std::mutex                         Logger::FileMutex;
std::mutex                         Logger::ConsoleMutex;

void Logger::GetConsoleLines(std::list<std::string>& lines)
{
	ConsoleMutex.lock();
	ConsoleStream->seekg(0, std::ios_base::beg);

	std::string line;
	while (std::getline(*ConsoleStream, line)) {
		lines.push_back(line);
	}
	ConsoleStream->clear();
	ConsoleMutex.unlock();
}

void Logger::InitLoggers(const char * pathFileLog)
{
	FileStream    = std::shared_ptr<std::fstream>(     new std::fstream(pathFileLog, std::ios::out | std::ios::trunc));
	ConsoleStream = std::shared_ptr<std::stringstream>(new std::stringstream(        std::ios::out | std::ios::in));
}

void Logger::LogAll(const char * str, ...)
{
	static char buff[2048];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	ConsoleMutex.lock();
	ConsoleStream->write(buff, strlen(buff));
	ConsoleStream->write("\n", 1);
	ConsoleStream->flush();
	ConsoleMutex.unlock();

	FileMutex.lock();
	FileStream->write(buff, strlen(buff));
	FileStream->write("\n", 1);
	FileStream->flush();
	FileMutex.unlock();
}

void Logger::Console(const char * str, ...)
{
	static char buff[2048];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	ConsoleMutex.lock();
	ConsoleStream->write(buff, strlen(buff));
	ConsoleStream->write("\n", 1);
	ConsoleStream->flush();
	ConsoleMutex.unlock();
}

void Logger::File(const char * str, ...)
{
	static char buff[2048];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	FileMutex.lock();
	FileStream->write(buff, strlen(buff));
	FileStream->write("\n", 1);
	FileStream->flush();
	FileMutex.unlock();
}




