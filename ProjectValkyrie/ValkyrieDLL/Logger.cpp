#include "Logger.h"
#include <stdarg.h>
#include "Valkyrie.h"

std::shared_ptr<std::fstream>      Logger::FileStream    = nullptr;
int                                Logger::BufferStart   = 0;
int                                Logger::BufferEnd = 0;
std::mutex                         Logger::LoggerMutex;
LogEntry                           Logger::Buffer[SIZE_LINE_BUFFER];

void Logger::IncrementBufferIndices()
{
	BufferEnd++;
	if (BufferEnd == SIZE_LINE_BUFFER)
		BufferEnd = 0;
	if (BufferEnd == BufferStart) {
		BufferStart++;
		if (BufferStart == SIZE_LINE_BUFFER)
			BufferStart = 0;
	}
}

void Logger::InitLoggers(const char * pathFileLog)
{
	FileStream = std::shared_ptr<std::fstream>(new std::fstream(pathFileLog, std::ios::out | std::ios::trunc));
}

int Logger::NextIndex(int currentIndex)
{
	currentIndex++;
	if (currentIndex == SIZE_LINE_BUFFER)
		currentIndex = 0;

	return currentIndex;
}

void Logger::Info(const char * str, ...)
{
	LoggerMutex.lock();

	IncrementBufferIndices();

	LogEntry& entry = Buffer[BufferEnd - 1];
	entry.type = LOG_INFO;

	va_list va;
	va_start(va, str);
	vsprintf_s(entry.message, str, va);
	va_end(va);

	FileStream->write("[info] ", 7);
	FileStream->write(entry.message, strlen(entry.message));
	FileStream->write("\n", 1);

	//FileStream->flush();
	LoggerMutex.unlock();
}

void Logger::Warn(const char * str, ...)
{
	LoggerMutex.lock();

	IncrementBufferIndices();

	LogEntry& entry = Buffer[BufferEnd - 1];
	entry.type = LOG_WARNING;

	va_list va;
	va_start(va, str);
	vsprintf_s(entry.message, str, va);
	va_end(va);

	FileStream->write("[warning] ", 10);
	FileStream->write(entry.message, strlen(entry.message));
	FileStream->write("\n", 1);

	LoggerMutex.unlock();
}

void Logger::Error(const char * str, ...)
{
	LoggerMutex.lock();

	IncrementBufferIndices();

	LogEntry& entry = Buffer[BufferEnd - 1];
	entry.type = LOG_ERROR;

	va_list va;
	va_start(va, str);
	vsprintf_s(entry.message, str, va);
	va_end(va);

	FileStream->write("[error] ", 8);
	FileStream->write(entry.message, strlen(entry.message));
	FileStream->write("\n", 1);

	LoggerMutex.unlock();
}


