#pragma once
#include <string>
#include <map>
#include <iostream>
#include <chrono>
#include <functional>

using namespace std::chrono;

/// Class used to save and load configs
class ConfigSet {

public:

	/// This constructor is declared for boost::python to be able to serialize this class
	ConfigSet();
	ConfigSet(std::string cfg, float saveInterval = 100);

	int            GetInt(const char* key, int defaultVal);
	bool           GetBool(const char* key, bool defaultVal);
	float          GetFloat(const char* key, float defaultVal);
	std::string    GetStr(const char*, const char* defaultVal);

	void           SetInt(const char*, int val);
	void           SetBool(const char*, bool val);
	void           SetFloat(const char*, float val);
	void           SetStr(const char*, const char* val);

	void           SetSaveInterval(float interval);
	void           SetConfigFile(std::string path);
	void           Reset();

	void           Load();
	void           Save();
	bool           IsTimeToSave();

private:
	float                              saveInterval;
	std::string                        filePath;
	std::map<std::string, std::string> rawValues;
	high_resolution_clock::time_point  timeLastSave;
};