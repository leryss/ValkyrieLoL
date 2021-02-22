#include "Globals.h"
#include "ValkyrieShared.h"
#include <fstream>

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

const fs::path     Globals::WorkingDir   = GetValkyrieFolder();
const fs::path     Globals::ConfigsDir   = GetConfigsDir();
const std::string  Globals::ImGuiIniPath = GetImGuiIniPath();