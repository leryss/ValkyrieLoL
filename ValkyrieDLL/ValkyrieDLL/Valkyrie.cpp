#include "Valkyrie.h"
#include "Strings.h"
#include "detours.h"
#include <stdexcept>
#include <iostream>

D3DPresentFunc                     Valkyrie::OriginalD3DPresent = NULL;

LPDIRECT3DDEVICE9                  Valkyrie::DxDevice           = NULL;
				                   
bool                               Valkyrie::Initialized        = false;


void Valkyrie::Run()
{
	try {
		Logger::FileLogger.Log("Starting up Valkyrie...\n");
		HookDirectX();
	}
	catch (std::exception& error) {
		Logger::FileLogger.Log("Failed starting up Valkyrie %s\n", error.what());
	}
}

void Valkyrie::ShowMenu()
{
	static bool ShowConsoleWindow = false;

	ImGui::Begin("Valkyrie", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::BeginMenu("Development")) {
		ImGui::Checkbox("Show Console", &ShowConsoleWindow);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Menu Settings")) {
		ImGui::ShowStyleSelector("Style");
		ImGui::EndMenu();
	}

	ImGui::End();

	if (ShowConsoleWindow)
		ShowConsole();
}

void Valkyrie::ShowConsole()
{
	ImGui::Begin("Console");
	
	auto consoleStream = std::dynamic_pointer_cast<std::stringstream>(Logger::ConsoleLogger.GetStream());
	consoleStream->clear();
	consoleStream->seekg(0);

	std::string line;
	while(std::getline(*consoleStream, line)) {
		ImGui::Text(line.c_str());
	} 
	
	ImGui::End();
}

void Valkyrie::InitializeOverlay()
{
	Logger::FileLogger.Log("Initializing overlay \n");

	HWND hWindow = FindWindowA("RiotWindowClass", NULL);
	SetWindowLongA(hWindow, GWL_WNDPROC, LONG_PTR(WindowMessageHandler));

	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(hWindow))
		throw std::runtime_error("Failed to initialize ImGui_ImplWin32_Init\n");

	if (!ImGui_ImplDX9_Init(DxDevice))
		throw std::runtime_error("Failed to initialize ImGui_ImplDX9_Init\n");
	
	Logger::ConsoleLogger.Log("Initialized Valkyrie Overlay!");
	Initialized = true;
}

void Valkyrie::Update()
{
	// Create frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ShowMenu();

	// Render
	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}


void Valkyrie::HookDirectX()
{
	static const int SearchLength       = 0x500000;
	static const int PresentVTableIndex = 17;
	static const int EndSceneVTableIndex = 42;

	Logger::FileLogger.Log("Hooking DirectX\n");

	DWORD objBase = (DWORD)LoadLibraryA("d3d9.dll");
	DWORD stopAt = objBase + SearchLength;

	Logger::FileLogger.Log("Found base of d3d9.dll at: %#010x\n", objBase);
	while (objBase++ < stopAt)
	{
		if ((*(WORD*)(objBase + 0x00)) == 0x06C7
			&& (*(WORD*)(objBase + 0x06)) == 0x8689
			&& (*(WORD*)(objBase + 0x0C)) == 0x8689
			) {
			objBase += 2;
			break;
		}
	}

	if (objBase >= stopAt)
		throw std::runtime_error("Did not find D3D device\n");
	Logger::FileLogger.Log("Found D3D Device at: %#010x\n", objBase);

	PDWORD VTable;
	*(DWORD*)& VTable = *(DWORD*)objBase;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OriginalD3DPresent = (D3DPresentFunc)(VTable[PresentVTableIndex]);
	LONG error = DetourAttach(&(PVOID&)OriginalD3DPresent, (PVOID)HookedD3DPresent);
	if (error)
		throw std::runtime_error(Strings::Format("Failed to hook DirectX present. Detours error code: %d"));

	DetourTransactionCommit();
	
}

void Valkyrie::UnhookDirectX()
{
	Logger::FileLogger.Log("Unhooking DirectX\n");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)OriginalD3DPresent, (PVOID)HookedD3DPresent);
	DetourTransactionCommit();
}

HRESULT __stdcall Valkyrie::HookedD3DPresent(LPDIRECT3DDEVICE9 Device, const RECT * pSrcRect, const RECT * pDestRect, HWND hDestWindow, const RGNDATA * pDirtyRegion)
{
	try {
		if (DxDevice != Device) {
			DxDevice = Device;
			InitializeOverlay();
		}
		Update();
	}
	catch (std::exception& error) {
		Logger::FileLogger.Log("Error occured %s\n", error.what());
		UnhookDirectX();
	}
	catch (...) {
		Logger::FileLogger.Log("Unexpected error occured.");
		UnhookDirectX();
	}

	return OriginalD3DPresent(Device, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

LRESULT ImGuiWindowMessageHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto& io = ImGui::GetIO();

	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		io.MouseDown[2] = true;
		return true;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return true;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = true;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = true;
		return true;
	case WM_XBUTTONUP:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = false;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}

	return 0;
}

LRESULT WINAPI Valkyrie::WindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGuiWindowMessageHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
