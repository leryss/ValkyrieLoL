#pragma once
#include <string>
#include <shlobj_core.h>

#include "miniz/miniz.h"
#include "imgui/imgui.h"

class ValkyrieShared {

public:
	static std::string GetWorkingDir() {
		char path[1024];
		if (!SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, TRUE))
			throw std::exception("Fatal error. Couldn't get appdata folder.");

		return std::string(path) + "\\" + "Valkyrie";
	}

	static void SaveCredentials(const char* name, const char* pass) {
		
		HKEY key;
		DWORD disposition;
		LSTATUS status;

		if (ERROR_SUCCESS != (status = RegCreateKeyExA(
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Valkyrie",
			NULL,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&key,
			&disposition))){

			auto err = std::string("Failed to open valkyrie registry ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		if (ERROR_SUCCESS != (status = RegSetKeyValueA(key, "", "username", REG_SZ, (const BYTE*)name, strlen(name)))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}
		if (ERROR_SUCCESS != (status = RegSetKeyValueA(key, "", "password", REG_SZ, (const BYTE*)pass, strlen(pass)))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		RegCloseKey(key);
	}

	static void LoadCredentials(char** name, char** pass) {
		HKEY key;
		DWORD disposition;
		LSTATUS status;

		if (ERROR_SUCCESS != (status = RegCreateKeyExA(
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Valkyrie",
			NULL,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&key,
			&disposition))) {

			auto err = std::string("Failed to open valkyrie registry ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		DWORD sizeName = 256;
		DWORD sizePass = 256;
		*name = new char[sizeName];
		*pass = new char[sizePass];

		if (ERROR_SUCCESS != (status = RegGetValueA(key, "", "username", RRF_RT_REG_SZ, NULL, *name, &sizeName))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}
		if (ERROR_SUCCESS != (status = RegGetValueA(key, "", "password", RRF_RT_REG_SZ, NULL, *pass, &sizePass))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		RegCloseKey(key);
	}
};