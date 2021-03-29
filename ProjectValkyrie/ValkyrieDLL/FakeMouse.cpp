#include "FakeMouse.h"
#include "Valkyrie.h"
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
		__try {
			return SpoofedGetCursorPos(lpPoint);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			Logger::Error("Error trying to get fake cursor position");
			return TRUE;
		}
	}
	else
		return TrueGetCursorPos(lpPoint);
}

BOOL __stdcall FakeMouse::SpoofedGetCursorPos(LPPOINT lpPoint)
{
	Vector2 v = FakeMouse::FakePositionGetter();
	lpPoint->x = (LONG)v.x + Valkyrie::WindowRect.left;
	lpPoint->y = (LONG)v.y + Valkyrie::WindowRect.top;

	return TRUE;
}
