#define STATUS_SUCCESS (0x00000000)

#include <codecvt>
#include <windows.h>
#include "Valkyrie.h"
#include "Globals.h"
#include "Strings.h"

bool CheckWindowsVersion() {
	HKEY currentVersion;
	LSTATUS status;
	if (ERROR_SUCCESS != (status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &currentVersion))) {
		Logger::Error("Failed to get Windows CurrentVersion: %d", status);
		return false;
	}

	DWORD valueType;
	BYTE buffer[256];
	DWORD bufferSize = 256;

	if (ERROR_SUCCESS != (status = RegQueryValueExA(currentVersion, "DisplayVersion", nullptr, &valueType, buffer, &bufferSize))) {
		Logger::Error("Failed to get Windows DisplayVersion: %d", status);
		return false;
	}

	Logger::Info("OS: %s", buffer);
}

DWORD WINAPI OverlayThreadEntryPoint(LPVOID lpParam) {

	fs::path pathFileLogger = Globals::WorkingDir;
	pathFileLogger.append("logs.txt");
	Logger::InitLoggers(pathFileLogger.u8string().c_str());

	CheckWindowsVersion();

	Logger::Info("Starting up Valkyrie");
	Valkyrie::Run();

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, &OverlayThreadEntryPoint, NULL, 0, NULL);

	return TRUE;
}

