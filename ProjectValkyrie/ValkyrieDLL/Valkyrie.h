#pragma once

#include <d3d9.h>
#include <dinput.h>
#include <windows.h>

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Logger.h"
#include "GameReader.h"
#include "ScriptManager.h"
#include "ConfigSet.h"
#include "InputController.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "ValkyrieAPI.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall * D3DPresentFunc)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
typedef HRESULT(__stdcall * BeginSceneFunc)(LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall * SetVertexShaderFunc)(LPDIRECT3DDEVICE9, IDirect3DVertexShader9*);
typedef HRESULT(__stdcall * SetTransformFunc)(LPDIRECT3DDEVICE9, D3DTRANSFORMSTATETYPE, const D3DMATRIX*);

class Valkyrie {

public:

	static void   Run();
	static void   WaitForOverlayToInit();

private:
	static bool                        CheckEssentialsLoaded();
	static void                        ShowMenu();
	static void                        ShowConsole();

	static void                        Update();
	static void                        InitializeOverlay();
	static void                        InitializePython();
	static void                        LoadScripts();
	static void                        ExecuteScripts();
	static void                        SetupScriptExecutionContext();
	static std::condition_variable     OverlayInitialized;

	static InputController             InputController;

	/// Game stuff
	static GameReader                  Reader;
	static ScriptManager               ScriptManager;
	static PyExecutionContext          ScriptContext;
	static HWND                        LeagueWindowHandle;

	/// DirectX stuff
	static void                        HookDirectX();
	static void                        UnhookDirectX();

	static HRESULT __stdcall           HookedD3DPresent(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion);
	static LRESULT WINAPI              HookedWindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static SetTransformFunc            OriginalSetTransform;
	static D3DPresentFunc              OriginalD3DPresent;
	static WNDPROC                     OriginalWindowMessageHandler;
	
	/// API
	static ValkyrieAPI                 Api;

public:
	static std::mutex                  DxDeviceMutex;
	static LPDIRECT3DDEVICE9           DxDevice;
	static GameState*                  CurrentGameState;
	static ConfigSet                   Configs;
};