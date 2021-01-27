#include "Memory.h"
#include "Strings.h"

std::string Memory::ReadString(int addr, int maxSize)
{
	const char* asciiName = (const char*)addr;

	std::string str("");
	if (Strings::ValidAsciiString(asciiName, maxSize))
		str.append(asciiName);

	return str;
}
