#include "Globals.h"
#include <fstream>

std::string FindGameVersion() {
	std::ifstream leagueCfg("../Config/game.cfg");

	std::string line;
	while (std::getline(leagueCfg, line)) {
		auto pos = line.find("=");
		if (pos != line.npos) {
			if (line.substr(0, pos).compare("CfgVersion") == 0)
				return line.substr(pos + 1, line.size());
		}
	}

	return std::string("Unknown");
}

fs::path    Globals::WorkingDir  = fs::path("F:\\Github\\ValkyrieLoL\\Deployable");
std::string Globals::GameVersion = FindGameVersion();
