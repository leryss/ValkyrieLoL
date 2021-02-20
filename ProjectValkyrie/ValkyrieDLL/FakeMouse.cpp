#include "FakeMouse.h"
#include "detours.h"

bool                      FakeMouse::Enabled = false;
std::function<Vector2()>  FakeMouse::FakePositionGetter;
GetCursorPosFunc          FakeMouse::TrueGetCursorPos = GetCursorPos;

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
		Vector2 v = FakeMouse::FakePositionGetter();
		lpPoint->x = v.x;
		lpPoint->y = v.y;

		return TRUE;
	}
	else
		return TrueGetCursorPos(lpPoint);
}