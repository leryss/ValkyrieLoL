#pragma once
#include <sstream>
#include <fstream>
#include <mutex>
#include <list>
#include <deque>

enum LogType {

	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO
};

class LogEntry {

public:
	LogType type;
	char    message[2048];
};

// Thread safe
class Logger {

private:
	static std::shared_ptr<std::fstream>      FileStream;
	static std::deque<std::string>            BufferDebug;
	static std::mutex                         LoggerMutex;

	static void IncrementBufferIndices();
public:
	/// Circular buffer for log lines
	static const int                          SIZE_LINE_BUFFER = 1024;
	static LogEntry                           Buffer[SIZE_LINE_BUFFER];
	static int                                BufferStart;
	static int                                BufferEnd;

	static void   InitLoggers(const char* pathFileLog);
	static int    NextIndex(int currentIndex);

	static void   Info(const char* str, ...);
	static void   Warn(const char* str, ...);
	static void   Error(const char* str, ...);

	static void   PushDebug(const char* str, ...);
	static void   DumpDebug();
	static void   ClearDebug();
};