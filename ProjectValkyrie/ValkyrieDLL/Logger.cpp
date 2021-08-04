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
	va_list va;
	va_start(va, str);
	WriteMessage(Color::WHITE, false, str, va);
	va_end(va);
}

void Logger::Warn(const char * str, ...)
{
	va_list va;
	va_start(va, str);
	WriteMessage(Color::YELLOW, false, str, va);
	va_end(va);
}

void Logger::Error(const char * str, ...)
{
	va_list va;
	va_start(va, str);
	WriteMessage(Color::RED, true, str, va);
	va_end(va);
}

void Logger::PushDebug(const char * str, ...)
{
	LoggerMutex.lock();
	char msg[4000];
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

void Logger::WriteMessage(const ImVec4& colorConsole, bool forceFlush, const char * formatStr, va_list formatArgs)
{
	static DWORD lastFlushTick = 0;

	LoggerMutex.lock();

	char msg[4000];
	vsprintf_s(msg, formatStr, formatArgs);

	*FileStream << msg << "\n";

	ConsoleStringLine* line = new ConsoleStringLine();
	line->text = msg;
	line->color = colorConsole;
	Valkyrie::Console.AddLine(std::shared_ptr<ConsoleLine>(line));

	if (forceFlush || GetTickCount() > lastFlushTick) {
		FileStream->flush();
		lastFlushTick = GetTickCount() + 100;
	}

	LoggerMutex.unlock();
}


