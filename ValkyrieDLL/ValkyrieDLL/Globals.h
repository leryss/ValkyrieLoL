#pragma once

#include <filesystem>

namespace fs = std::experimental::filesystem;

class Globals {
public:
	static fs::path    WorkingDir;
	static fs::path    ConfigsDir;
};