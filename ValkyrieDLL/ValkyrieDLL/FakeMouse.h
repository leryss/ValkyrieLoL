#pragma once
#include "Vector.h"
#include <windows.h>

typedef BOOL(WINAPI* GetCursorPosFunc)(LPPOINT lpPoint);

class FakeMouse {

public:
	static void    Init();
	static bool    Enabled;
	static Vector2 FakePosition;

private:
	static GetCursorPosFunc  TrueGetCursorPos;
	static BOOL __stdcall    HookedGetCursorPos(LPPOINT lpPoint);
};