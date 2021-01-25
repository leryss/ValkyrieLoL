#pragma once

#include <d3d9.h>
#include <dinput.h>
#include <windows.h>
#include <mutex>
#include <sstream>

#include "Logger.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall* D3DPresentFunc)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);

class Valkyrie {

public:

	void   Run();
private:

	static void                        ShowMenu();
	static void                        ShowConsole();

	static void                        Update();
	static void                        InitializeOverlay();
	static bool                        Initialized;

	// DirectX stuff
	static void                        HookDirectX();
	static void                        UnhookDirectX();

	static LRESULT WINAPI              WindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static HRESULT __stdcall           HookedD3DPresent(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion);

	static D3DPresentFunc              OriginalD3DPresent;
	static LPDIRECT3DDEVICE9           DxDevice;
};