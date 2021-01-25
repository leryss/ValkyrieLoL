#include <windows.h>
#include "Valkyrie.h"
#include "Strings.h"


DWORD WINAPI OverlayThreadEntryPoint(LPVOID lpParam) {
	
	Valkyrie valkyrie;
	valkyrie.Run();

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

