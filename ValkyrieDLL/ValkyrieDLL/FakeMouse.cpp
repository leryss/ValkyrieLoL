#include "FakeMouse.h"
#include "detours.h"

bool             FakeMouse::Enabled = false;
Vector2          FakeMouse::FakePosition;
GetCursorPosFunc FakeMouse::TrueGetCursorPos = GetCursorPos;

void FakeMouse::Init() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (DetourAttach(&(PVOID&)TrueGetCursorPos, (PVOID)HookedGetCursorPos)) {
		throw std::runtime_error("Failed to Hook GetCursorPos");
	}

	DetourTransactionCommit();
}

BOOL __stdcall FakeMouse::HookedGetCursorPos(LPPOINT lpPoint)
{
	if (lpPoint != NULL && FakeMouse::Enabled) {
		lpPoint->x = FakeMouse::FakePosition.x;
		lpPoint->y = FakeMouse::FakePosition.y;

		return TRUE;
	}
	else
		return TrueGetCursorPos(lpPoint);
}