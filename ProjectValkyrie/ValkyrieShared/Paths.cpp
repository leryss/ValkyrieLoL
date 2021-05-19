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
std::string Paths::Configs      = Root + "\\configs";
std::string Paths::ImguiConfig  = Root + "\\imgui.ini";
std::string Paths::PredictionNN = Root + "\\data\\Prediction.valk";

bool Paths::FileExists(std::string & path)
{
	DWORD dwAttrib = GetFileAttributesA(path.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::string Paths::GetScriptPath(std::string & scriptName)
{
	std::string res;
	res.append(Scripts);
	res.append("\\");
	res.append(scriptName);
	res.append(".py");

	return res;
}

std::string Paths::GetTemporaryPath() {
	
	char path[MAX_PATH];
	GetTempPathA(MAX_PATH, path);

	return std::string(path);
}
