#pragma once
#include "Vector.h"
#include <windows.h>
#include <functional>

typedef BOOL(WINAPI* GetCursorPosFunc)(LPPOINT lpPoint);

/// This utility fakes mouse position by hooking the WINAPI GetCursorPos function
class FakeMouse {

public:
	/// Initializes the utility & hooks the necessary function for faking mouse position.
	static void                     Init();

	/// True if the mouse position should be faked
	static bool                     Enabled;

	/// Getter for the fake position
	static std::function<Vector2()> FakePositionGetter;

private:
	static GetCursorPosFunc  TrueGetCursorPos;

	static BOOL __stdcall    HookedGetCursorPos(LPPOINT lpPoint);
	static BOOL __stdcall    SpoofedGetCursorPos(LPPOINT lpPoint);
};