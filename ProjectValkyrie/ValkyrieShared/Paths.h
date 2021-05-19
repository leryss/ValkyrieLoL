#pragma once
#include <string>
#include <shlobj_core.h>
#undef GetObject
#undef min
#undef max

class Paths {

public:

	static bool FileExists(std::string& path);
	static std::string GetScriptPath(std::string& scriptName);
	static std::string GetTemporaryPath();

	static std::string Root;
	static std::string Payload;
	static std::string Scripts;
	static std::string ScriptsIndex;
	static std::string ChangeLog;
	static std::string Dependencies;
	static std::string Version;
	static std::string Configs;
	static std::string ImguiConfig;
	static std::string PredictionNN;
};