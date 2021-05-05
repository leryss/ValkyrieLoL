#include "DirectInputHook.h"
#include "detours.h"
#include "Logger.h"
#include "Strings.h"

bool                                DirectInputHook::DisableGameKeys = false;
DirectInputGetDeviceData            DirectInputHook::OriginalDirectInputGetDeviceData;
std::set<HKey>                      DirectInputHook::DisabledGameKeys;
std::queue<std::pair<DWORD, DWORD>> DirectInputHook::EventQueue;

void DirectInputHook::Hook()
{
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	IDirectInput8 *pDirectInput = NULL;

	if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
		Logger::Error("Failed to hook kbd input. Couldnt create DirectInput");
	}

	LPDIRECTINPUTDEVICE8  lpdiKeyboard;
	if (pDirectInput->CreateDevice(GUID_SysKeyboard, &lpdiKeyboard, NULL) != DI_OK)
	{
		pDirectInput->Release();
		Logger::Error("Failed to hook kbd input. Couldnt create DirectInput device");
		return;
	}

	void ** VTable = *reinterpret_cast<void***>(lpdiKeyboard);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OriginalDirectInputGetDeviceData = (DirectInputGetDeviceData)(VTable[10]);
	LONG error = DetourAttach(&(PVOID&)OriginalDirectInputGetDeviceData, (PVOID)HookedDirectInputGetDeviceData);
	if (error)
		throw std::runtime_error(Strings::Format("DetourAttach: Failed to hook DirectInput GetDeviceData. Detours error code: %d", error));

	error = DetourTransactionCommit();
	if (error)
		throw std::runtime_error(Strings::Format("DetourCommitTransaction: Failed to hook DirectInput GetDeviceData. Detours error code: %d", error));
}

void DirectInputHook::QueueKey(HKey key, bool pressed)
{
	EventQueue.push(std::pair<DWORD, DWORD>({ key, (pressed ? LOBYTE(0x80) : 0) }));
}

void DirectInputHook::SetKeyActive(HKey key, bool active)
{
	if (active)
		DisabledGameKeys.erase(key);
	else
		DisabledGameKeys.insert(key);
}

HRESULT __stdcall DirectInputHook::HookedDirectInputGetDeviceData(IDirectInputDevice8 * pThis, DWORD a, LPDIDEVICEOBJECTDATA data, LPDWORD numElemsPtr, DWORD d)
{
	auto result = OriginalDirectInputGetDeviceData(pThis, a, data, numElemsPtr, d);
	if (result != DI_OK)
		return result;

	int numElems = *numElemsPtr;
	for (int i = 0; i < numElems; ++i) {
		HKey key = (HKey)data[i].dwOfs;
		auto find = DisabledGameKeys.find(key);
		if (find != DisabledGameKeys.end()) {
			data[i].dwOfs = 0;
		}
	}

	while (!EventQueue.empty()) {
		auto pair = EventQueue.front();

		data[numElems].dwData = pair.second;
		data[numElems].dwOfs = pair.first;
		numElems++;

		EventQueue.pop();
	}

	*numElemsPtr = numElems;

	return DI_OK;
	//if (DisableGameKeys) {
	//	*numElems = 0;
	//	return DI_OK;
	//}
}
