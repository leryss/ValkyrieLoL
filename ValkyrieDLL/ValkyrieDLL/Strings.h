#pragma once
#include <string>

class Strings {

public:
	static bool        ContainsOnlyASCII(const char* buff, int maxSize);
	static std::string ToLower(std::string str);
	static std::string RandomString(const int len);
	static std::string Format(const char* c, ...);
};