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

std::string GetImGuiIniPath() {
	
	fs::path path = Globals::WorkingDir;
	path /= "imgui.ini";

	return path.u8string();
}

const fs::path     Globals::WorkingDir   = FindWorkingDir();
const fs::path     Globals::ConfigsDir   = GetConfigsDir();
const std::string  Globals::ImGuiIniPath = GetImGuiIniPath();