#pragma once
#include "HKey.h"
#include <set>
#include <queue>
#include <dinput.h>

typedef HRESULT(WINAPI* DirectInputGetDeviceData)(IDirectInputDevice8*, DWORD a, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);

class DirectInputHook {

public:
	static void Hook();
	static void QueueKey(HKey key, bool pressed);
	static void SetKeyActive(HKey key, bool active);

	static HRESULT WINAPI HookedDirectInputGetDeviceData(IDirectInputDevice8* pThis, DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d);

public:
	static bool                        DisableGameKeys;

private:
	static DirectInputGetDeviceData    OriginalDirectInputGetDeviceData;
	static std::set<HKey>              DisabledGameKeys;
	static std::queue<std::pair<DWORD, DWORD>> EventQueue;
};