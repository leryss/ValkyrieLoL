#include "DirectInputHook.h"
#include "detours.h"

#include "Strings.h"

bool                                DirectInputHook::DisableGameKeys = false;
DirectInputGetDeviceData            DirectInputHook::OriginalDirectInputGetDeviceData;
std::set<DWORD>                     DirectInputHook::DisabledGameKeys;
std::map<DWORD, InputEventInfo>     DirectInputHook::AdditionalEvents;
DWORD                               DirectInputHook::SequenceNumber;

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
	AdditionalEvents[key] = { 
		ConvertToDIKey(key),
		pressed ? 0x80u : 0u, 
		GetTickCount(),
		SequenceNumber++,
		pressed 
	};
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

	if (DisableGameKeys) {
		*numElemsPtr = 0;
		return DI_OK;
	}

	/// Check disabled keys
	int numElems = *numElemsPtr;
	if (!GetAsyncKeyState(VK_CONTROL)) {
		for (int i = 0; i < numElems; ++i) {
			auto find = DisabledGameKeys.find(data[i].dwOfs);
			if (find != DisabledGameKeys.end()) {
				data[i].dwOfs = 0;
			}
		}
	}

	/// Update sequence number
	if (numElems > 0)
		SequenceNumber = data[numElems - 1].dwSequence + 1;
	
	/// Add additional fake inputs
	auto it = AdditionalEvents.begin();
	while (it != AdditionalEvents.end()) {
		auto event = it->second;

		data[numElems].dwData = event.data;
		data[numElems].dwOfs = event.offset;
		data[numElems].dwSequence = event.sequence;
		data[numElems].dwTimeStamp = event.timestamp;
		numElems++;

		it = AdditionalEvents.erase(it);
	}

	*numElemsPtr = numElems;

	return DI_OK;
}

DWORD DirectInputHook::ConvertToDIKey(HKey key)
{
	switch (key) {
	case MOUSE_BTN:
		return 260;
		break;
	case MOUSE_BTN_2:
		return 261;
		break;
	default:
		return key;
	}
}
