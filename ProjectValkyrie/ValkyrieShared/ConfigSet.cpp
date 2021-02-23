#include "ConfigSet.h"
#include <fstream>

void ConfigSet::Load() {
	std::string line;
	size_t delimiterIdx;

	std::ifstream file(filePath, std::ios::in);
	if (file.is_open()) {
		while (std::getline(file, line)) {
			delimiterIdx = line.find("=");
			if (delimiterIdx == std::string::npos)
				throw std::runtime_error(std::string("Config line does not contain delimiter `=`: ").append(line));

			rawValues[line.substr(0, delimiterIdx)] = line.substr(delimiterIdx + 1, line.length());
		}
	}
}

void ConfigSet::Save() {

	std::ofstream file(filePath, std::ios::trunc | std::ios::out);
	if (!file.is_open())
		throw std::runtime_error("Couldn't open file to save config set");

	for (auto it = rawValues.begin(); it != rawValues.end(); ++it) {
		file << it->first << "=" << it->second << "\n";
	}

	file.close();
	timeLastSave = high_resolution_clock::now();
}

bool ConfigSet::IsTimeToSave()
{
	duration<float, std::milli> dur = high_resolution_clock::now() - timeLastSave;
	return dur.count() > saveInterval;
}

ConfigSet::ConfigSet()
{
}

ConfigSet::ConfigSet(std::string cfg, float saveInterval)
{
	SetSaveInterval(saveInterval);
	SetConfigFile(cfg);
}

int ConfigSet::GetInt(const char* key, int defaultVal) {
	auto it = rawValues.find(key);
	if (it == rawValues.end())
		return defaultVal;

	std::string& val = it->second;

	if (val.substr(0, 2).compare("0x") == 0)
		return std::stoi(val, nullptr, 16);
	return std::stoi(val);
}

float ConfigSet::GetFloat(const char* key, float defaultVal) {
	auto it = rawValues.find(key);
	if (it == rawValues.end())
		return defaultVal;

	std::string& val = it->second;

	return std::stof(val);
}

bool ConfigSet::GetBool(const char* key, bool defaultVal) {
	auto it = rawValues.find(key);
	if (it == rawValues.end())
		return defaultVal;

	std::string& val = it->second;

	return std::stod(val);
}

std::string ConfigSet::GetStr(const char* key, const char* defaultVal) {
	auto it = rawValues.find(key);
	if (it == rawValues.end())
		return defaultVal;

	return it->second;
}

void ConfigSet::SetInt(const char* key, int value) {
	rawValues[std::string(key)] = std::to_string(value);
}

void ConfigSet::SetFloat(const char* key, float value) {
	rawValues[std::string(key)] = std::to_string(value);
}

void ConfigSet::SetBool(const char* key, bool value) {
	rawValues[std::string(key)] = std::to_string(value);
}

void ConfigSet::SetStr(const char* key, const char* value) {
	rawValues[std::string(key)] = value;
}

void ConfigSet::SetSaveInterval(float interval)
{
	saveInterval = interval;
}

void ConfigSet::SetConfigFile(std::string path)
{
	filePath = path;
}
