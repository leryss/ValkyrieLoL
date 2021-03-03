#define STATUS_SUCCESS (0x00000000)

#include <codecvt>
#include <windows.h>
#include "Valkyrie.h"
#include "Strings.h"
#include "Paths.h"


DWORD WINAPI OverlayThreadEntryPoint(LPVOID lpParam) {

	auto pathFileLogger = Paths::Root;
	pathFileLogger.append("\\");
	pathFileLogger.append("logs.txt");
	Logger::InitLoggers(pathFileLogger.c_str());

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

