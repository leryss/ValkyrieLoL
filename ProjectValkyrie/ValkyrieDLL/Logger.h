#pragma once
#include <sstream>
#include <fstream>
#include <mutex>
#include <list>
#include <deque>

/// Thread safe logging utility
class Logger {

private:
	static std::shared_ptr<std::fstream>      FileStream;
	static std::deque<std::string>            BufferDebug;
	static std::mutex                         LoggerMutex;

public:

	/// This must be called before using the logger. It initializes the necessary buffers and creates the log file
	static void   InitLoggers(const char* pathFileLog);

	/// Writes a info log message to file and to the log memory buffer
	static void   Info(const char* str, ...);
	
	/// Writes a warn log message to file and to the log memory buffer
	static void   Warn(const char* str, ...);
	
	/// Writes a error log message to file and to the log memory buffer. This function flushes the file stream to avoid missing logs in case of a crash.
	static void   Error(const char* str, ...);

	/// Pushes a debug message. Debug messages are held in a different buffer and they must be flushed manually to file
	static void   PushDebug(const char* str, ...);

	/// Flushes the debug messages to file
	static void   DumpDebug();

	/// Clears the debug message buffer
	static void   ClearDebug();

private:

	static void   WriteMessage(const ImVec4& colorConsole, bool forceFlush, const char * formatStr, va_list formatArgs);
};