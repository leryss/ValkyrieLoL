#include "Strings.h"
#include <ctime>
#include <algorithm>
#include <stdarg.h>

bool Strings::ValidAsciiString(const char* buff, int maxSize) {
	for (int i = 0; i < maxSize; ++i) {
		if (buff[i] == 0)
			return true;
		
		unsigned char c = (unsigned char)buff[i];
		if (c > 127 || c < 33)
			return false;
	}
	return false;
}

std::string Strings::ToLower(std::string& str)
{
	std::string strLower;
	strLower.resize(str.size());

	std::transform(str.begin(),
		str.end(),
		strLower.begin(),
		::tolower);

	return strLower;
}

std::string Strings::Format(const char* c, ...) {
	char buff[200];
	va_list va;
	va_start(va, c);
	vsprintf_s(buff, c, va);
	va_end(va);

	return std::string(buff);
}

std::string Strings::RandomString(const int len) {

	std::string tmp_s;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	srand((unsigned int)time(0));
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];


	return tmp_s;
}