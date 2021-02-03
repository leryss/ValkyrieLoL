#pragma once

#include <filesystem>

namespace fs = std::experimental::filesystem;

class Globals {
public:
	const static fs::path      WorkingDir;
	const static fs::path      ConfigsDir;
	const static std::string   ImGuiIniPath;
};