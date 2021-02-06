#pragma once
#include "Vector.h"
#include <windows.h>
#include <functional>

typedef BOOL(WINAPI* GetCursorPosFunc)(LPPOINT lpPoint);

class FakeMouse {

public:
	static void                     Init();
	static bool                     Enabled;
	static std::function<Vector2()> FakePositionGetter;

private:
	static GetCursorPosFunc  TrueGetCursorPos;
	static BOOL __stdcall    HookedGetCursorPos(LPPOINT lpPoint);
};