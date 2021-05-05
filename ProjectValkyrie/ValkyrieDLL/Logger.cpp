#include "Logger.h"
#include <stdarg.h>
#include "Valkyrie.h"

std::shared_ptr<std::fstream>      Logger::FileStream    = nullptr;
std::mutex                         Logger::LoggerMutex;
std::deque<std::string>            Logger::BufferDebug;

void Logger::InitLoggers(const char * pathFileLog)
{
	FileStream = std::shared_ptr<std::fstream>(new std::fstream(pathFileLog, std::ios::out | std::ios::trunc));
}

void Logger::Info(const char * str, ...)
{
	LoggerMutex.lock();

	char message[2048];

	va_list va;
	va_start(va, str);
	vsprintf_s(message, str, va);
	va_end(va);

	ConsoleStringLine* line = new ConsoleStringLine();
	line->text = message;
	line->color = Color::WHITE;
	Valkyrie::Console.AddLine(std::shared_ptr<ConsoleLine>(line));

	//FileStream->flush();
	LoggerMutex.unlock();
}

void Logger::Warn(const char * str, ...)
{
	LoggerMutex.lock();

	char message[2048];

	va_list va;
	va_start(va, str);
	vsprintf_s(message, str, va);
	va_end(va);

	ConsoleStringLine* line = new ConsoleStringLine();
	line->text = message;
	line->color = Color::YELLOW;
	Valkyrie::Console.AddLine(std::shared_ptr<ConsoleLine>(line));

	LoggerMutex.unlock();
}

void Logger::Error(const char * str, ...)
{
	LoggerMutex.lock();

	char message[2048];

	va_list va;
	va_start(va, str);
	vsprintf_s(message, str, va);
	va_end(va);

	ConsoleStringLine* line = new ConsoleStringLine();
	line->text = message;
	line->color = Color::RED;
	Valkyrie::Console.AddLine(std::shared_ptr<ConsoleLine>(line));

	FileStream->flush(); // We flush when we get errors to make sure we got them on logs.txt

	LoggerMutex.unlock();
}

void Logger::PushDebug(const char * str, ...)
{
	LoggerMutex.lock();
	char msg[512];
	va_list va;
	va_start(va, str);
	vsprintf_s(msg, str, va);
	va_end(va);
	
	BufferDebug.push_back(std::string(msg));
	LoggerMutex.unlock();
}

void Logger::DumpDebug()
{
	LoggerMutex.lock();
	for (auto msg : BufferDebug) {
		FileStream->write("[debug] ", 8);
		FileStream->write(msg.c_str(), msg.size());
		FileStream->write("\n", 1);
	}
	LoggerMutex.unlock();
}

void Logger::ClearDebug()
{
	LoggerMutex.lock();
	BufferDebug.clear();
	LoggerMutex.unlock();
}


