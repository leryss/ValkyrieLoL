#include "Valkyrie.h"
#include "Strings.h"
#include "detours.h"
#include "GameData.h"
#include "ObjectExplorer.h"
#include "Offsets.h"
#include "Memory.h"
#include "Globals.h"
#include "PyStructs.h"

#include <stdexcept>
#include <iostream>

D3DPresentFunc                     Valkyrie::OriginalD3DPresent           = NULL;
WNDPROC                            Valkyrie::OriginalWindowMessageHandler = NULL;
LPDIRECT3DDEVICE9                  Valkyrie::DxDevice           = NULL;
std::mutex                         Valkyrie::DxDeviceMutex;

std::condition_variable            Valkyrie::OverlayInitialized;

GameReader                         Valkyrie::Reader;
PyExecutionContext                 Valkyrie::ScriptContext;
ScriptManager                      Valkyrie::ScriptManager;


void Valkyrie::Run()
{
	try {
		DxDeviceMutex.lock();

		GameData::LoadAsync();
		HookDirectX();
	}
	catch (std::exception& error) {
		Logger::File.Log("Failed starting up Valkyrie %s", error.what());
	}
}

void Valkyrie::WaitForOverlayToInit()
{
	std::mutex mtx;
	std::unique_lock<std::mutex> lock(mtx);
	Valkyrie::OverlayInitialized.wait(lock);
}

bool Valkyrie::CheckEssentialsLoaded()
{
	if(!GameData::LoadProgress->allLoaded)
		GameData::ImGuiDrawLoader();

	if (GameData::LoadProgress->essentialsLoaded)
		return true;
	return false;
}

void Valkyrie::ShowMenu(GameState& state)
{
	static bool ShowConsoleWindow        = true;
	static bool ShowObjectExplorerWindow = true;

	ImGui::Begin("Valkyrie", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::BeginMenu("Development")) {

		if (ImGui::Button("Reload Scripts"))
			LoadScripts();

		ImGui::Checkbox("Show Console", &ShowConsoleWindow);
		ImGui::Checkbox("Show Object Explorer", &ShowObjectExplorerWindow);
		if (ImGui::TreeNode("Benchmarks")) {
		
			Reader.GetBenchmarks().ImGuiDraw();
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Menu Settings")) {
		ImGui::ShowStyleSelector("Style");
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Skin Changer")) {
		static int skinId;
		static int objAddr;
		ImGui::DragInt("Obj", &objAddr);
		ImGui::DragInt("SkinId", &skinId);

		if (ImGui::Button("Change")) {
			int baseAddr = (int)GetModuleHandle(NULL);
			auto Update = reinterpret_cast<void(__thiscall*)(void*, bool)>(baseAddr + Offsets::FnCharacterDataStackUpdate);

			int charDataStack = objAddr + Offsets::CharacterDataStack;
			int* charSkinId = (int*)(charDataStack + Offsets::CharacterDataStackSkinId);

			*charSkinId = skinId;
			Update((void*)charDataStack, true);
		}

		ImGui::EndMenu();
	}

	ImGui::Separator();
	if(state.gameStarted)
		ScriptManager.ImGuiDrawMenu(ScriptContext);

	ImGui::End();

	if (ShowConsoleWindow)
		ShowConsole();

	if (ShowObjectExplorerWindow)
		ObjectExplorer::ImGuiDraw(state);
}

void Valkyrie::ShowConsole()
{
	ImGui::Begin("Console");
	
	std::list<std::string> lines;
	Logger::Console.GetLines(lines);

	for (auto& line : lines) {
		ImGui::Text(line.c_str());
	}
	
	ImGui::End();
}

void Valkyrie::InitializeOverlay()
{
	Logger::LogAll("Initializing overlay");

	HWND hWindow = FindWindowA("RiotWindowClass", NULL);
	OriginalWindowMessageHandler = WNDPROC(SetWindowLongA(hWindow, GWL_WNDPROC, LONG_PTR(HookedWindowMessageHandler)));

	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(hWindow))
		throw std::runtime_error("Failed to initialize ImGui_ImplWin32_Init");

	if (!ImGui_ImplDX9_Init(DxDevice))
		throw std::runtime_error("Failed to initialize ImGui_ImplDX9_Init");
	
	OverlayInitialized.notify_all();
}

void Valkyrie::InitializePython()
{
	Logger::LogAll("Initializing Python");
	PyImport_AppendInittab("valkyrie", &PyInit_valkyrie);
	Py_Initialize();
}

void Valkyrie::LoadScripts()
{
	fs::path pathScripts = Globals::WorkingDir;
	pathScripts.append("scripts");
	std::string pathStr = pathScripts.u8string();

	ScriptManager.LoadScriptsFromFolder(pathStr);
}

void Valkyrie::Update()
{
	// Create frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (CheckEssentialsLoaded()) {
		GameState& state = Reader.GetNextState();
		if(state.gameStarted)
			ScriptManager.ExecuteScripts(ScriptContext);
		ShowMenu(state);
	}

	// Render
	ImGui::EndFrame();
	ImGui::Render();

	DxDeviceMutex.lock();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	DxDeviceMutex.unlock();
}

void Valkyrie::HookDirectX()
{
	static const int SearchLength       = 0x500000;
	static const int PresentVTableIndex = 17;
	static const int EndSceneVTableIndex = 42;

	Logger::LogAll("Hooking DirectX");

	DWORD objBase = (DWORD)LoadLibraryA("d3d9.dll");
	DWORD stopAt = objBase + SearchLength;

	Logger::File.Log("Found base of d3d9.dll at: %#010x", objBase);
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
		throw std::runtime_error("Did not find D3D device");
	Logger::File.Log("Found D3D Device at: %#010x", objBase);

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
	Logger::File.Log("Unhooking DirectX");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)OriginalD3DPresent, (PVOID)HookedD3DPresent);
	DetourTransactionCommit();
}

HRESULT __stdcall Valkyrie::HookedD3DPresent(LPDIRECT3DDEVICE9 Device, const RECT * pSrcRect, const RECT * pDestRect, HWND hDestWindow, const RGNDATA * pDirtyRegion)
{
	DxDeviceMutex.unlock();

	try {
		if (DxDevice == NULL) {
			DxDevice = Device;
			InitializePython();
			InitializeOverlay();
			LoadScripts();
		}
		Update();
	}
	catch (std::exception& error) {
		Logger::File.Log("Error occured %s", error.what());
		UnhookDirectX();
	}
	catch (...) {
		Logger::File.Log("Unexpected error occured.");
		UnhookDirectX();
	}

	DxDeviceMutex.lock();

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

LRESULT WINAPI Valkyrie::HookedWindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	return CallWindowProcA(OriginalWindowMessageHandler, hWnd, msg, wParam, lParam);
}
