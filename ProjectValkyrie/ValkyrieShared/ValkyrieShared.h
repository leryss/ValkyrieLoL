#pragma once
#include <string>
#include <shlobj_core.h>

std::string GetValkyrieFolder() {
	char path[1024];
	if (!SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, TRUE))
		throw std::exception("Fatal error. Couldn't get appdata folder.");

	return std::string(path) + "\\" + "Valkyrie";
}