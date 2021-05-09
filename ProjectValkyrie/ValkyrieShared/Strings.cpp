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

std::string Strings::ToLower(const std::string& str)
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
	char buff[2048];
	va_list va;
	va_start(va, c);
	vsprintf_s(buff, c, va);
	va_end(va);

	return std::string(buff);
}

std::string Strings::RandomAsciiLowerString(const int len) {

	std::string tmp_s;
	static const char alpha[] = "abcdefghijklmnopqrstuvwxyz";

	srand((unsigned int)time(0));
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += alpha[rand() % (sizeof(alpha) - 1)];


	return tmp_s;
}

std::string Strings::RandomDLLName()
{
	srand((unsigned int)time(0));
	int sizeAlpha = 5 + (rand() % 4);
	bool use32 = rand() % 2;

	return Format("%s%s.dll", RandomAsciiLowerString(sizeAlpha).c_str(), (use32 ? "32" : ""));
}
