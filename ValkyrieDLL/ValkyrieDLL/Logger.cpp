#include "Logger.h"
#include <stdarg.h>
#include "Valkyrie.h"

Logger Logger::FileLogger(
	std::shared_ptr<std::fstream>(new std::fstream("logs.txt", std::ios::out | std::ios::trunc))
);

Logger Logger::ConsoleLogger(
	std::shared_ptr<std::stringstream>(new std::stringstream(std::ios::out | std::ios::in))
);

Logger::Logger(std::shared_ptr<std::iostream> stream)
{
	this->stream = stream;
}

void Logger::Log(const char * str, ...)
{
	char buff[200];
	va_list va;
	va_start(va, str);
	vsprintf_s(buff, str, va);
	va_end(va);

	stream->write(buff, strlen(buff));
	stream->flush();
}

std::shared_ptr<std::iostream> Logger::GetStream()
{
	return stream;
}
