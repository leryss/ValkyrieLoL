#include "Logger.h"
#include <stdarg.h>
#include "Valkyrie.h"

Logger Logger::File(
	std::shared_ptr<std::fstream>(new std::fstream("logs.txt", std::ios::out | std::ios::trunc))
);

Logger Logger::Console(
	std::shared_ptr<std::stringstream>(new std::stringstream(std::ios::out | std::ios::in))
);

Logger::Logger(std::shared_ptr<std::iostream> stream)
{
	this->stream = stream;
}

void Logger::Log(const char * str, ...)
{
	static char buff[2048];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	streamMutex.lock();
	stream->write(buff, strlen(buff));
	stream->write("\n", 1);
	stream->flush();
	streamMutex.unlock();
}

void Logger::GetLines(std::list<std::string>& lines)
{
	streamMutex.lock();
	stream->seekg(0, std::ios_base::beg);

	std::string line;
	while (std::getline(*stream, line)) {
		lines.push_back(line);
	}
	stream->clear();
	streamMutex.unlock();
}

void Logger::LogAll(const char * str, ...)
{
	static char buff[2048];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	File.Log(buff);
	Console.Log(buff);
}
