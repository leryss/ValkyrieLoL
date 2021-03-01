#include "Paths.h"

std::string CalculateRootPath() {
	char path[1024];
	if (!SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, TRUE))
		throw std::exception("Fatal error. Couldn't get appdata folder.");

	return std::string(path) + "\\" + "Valkyrie";
}

std::string Paths::Root         = CalculateRootPath();
std::string Paths::Payload      = Root + "\\payload\\ValkyrieDLL.dll";
std::string Paths::Scripts      = Root + "\\scripts";
std::string Paths::ScriptsIndex = Root + "\\scripts.index";
std::string Paths::ChangeLog    = Root + "\\changelog.txt";
std::string Paths::Dependencies = Root + "\\dependencies";
std::string Paths::Version      = Root + "\\version";