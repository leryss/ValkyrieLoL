#include "Globals.h"
#include <fstream>

fs::path FindWorkingDir() {
	char* buff;
	size_t buffCount;
	_dupenv_s(&buff, &buffCount, "VPath");

	return fs::path(buff);
}

fs::path GetConfigsDir() {

	fs::path path = Globals::WorkingDir;
	path /= "configs";
	
	return path;
}

fs::path    Globals::WorkingDir  = FindWorkingDir();
fs::path    Globals::ConfigsDir  = GetConfigsDir();
