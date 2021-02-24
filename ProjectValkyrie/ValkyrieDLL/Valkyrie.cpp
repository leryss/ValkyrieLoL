#include "Valkyrie.h"
#include "Strings.h"
#include "detours.h"
#include "GameData.h"
#include "ObjectExplorer.h"
#include "Offsets.h"
#include "Memory.h"
#include "Globals.h"
#include "PyStructs.h"
#include "OffsetScanner.h"
#include "SkinChanger.h"
#include "FakeMouse.h"
#include "ValkyrieShared.h"

#include "D3DX9Shader.h"

#include <boost/exception/diagnostic_information.hpp>

#include <stdexcept>
#include <iostream>
#include <functional>

D3DPresentFunc                     Valkyrie::OriginalD3DPresent           = NULL;
WNDPROC                            Valkyrie::OriginalWindowMessageHandler = NULL;
LPDIRECT3DDEVICE9                  Valkyrie::DxDevice                     = NULL;
std::mutex                         Valkyrie::DxDeviceMutex;
HWND                               Valkyrie::LeagueWindowHandle;

ValkyrieAPI                        Valkyrie::Api;
UserInfo                           Valkyrie::LoggedUser;
AsyncTaskPool                      Valkyrie::TaskPool;
bool                               Valkyrie::EssentialsLoaded = false;

GameReader                         Valkyrie::Reader;
PyExecutionContext                 Valkyrie::ScriptContext;
ScriptManager                      Valkyrie::ScriptManager;
GameState*                         Valkyrie::CurrentGameState = NULL;

InputController                    Valkyrie::InputController;
ConfigSet                          Valkyrie::Configs;
bool                               Valkyrie::ShowConsoleWindow;
bool                               Valkyrie::ShowObjectExplorerWindow;
bool                               Valkyrie::ShowOffsetScanner;
HKey                               Valkyrie::ShowMenuKey;
int                                Valkyrie::MenuStyle;
float                              Valkyrie::AveragePing;

bool ChooseMenuStyle(const char* label, int& currentStyle)
{
	return ImGui::Combo(label, &currentStyle, "Dark\0Light\0Classic\0");
}

int SetStyle(int style) {
	switch (style)
	{
	case 0: ImGui::StyleColorsDark(); break;
	case 1: ImGui::StyleColorsLight(); break;
	case 2: ImGui::StyleColorsClassic(); break;
	}
	return style;
}

void Valkyrie::Run()
{
	try {
		DxDeviceMutex.lock();

		FakeMouse::Init();
		HookDirectX();
		LoginAndLoadData();
	}
	catch (std::exception& error) {
		Logger::Error("Failed starting up Valkyrie %s", error.what());
	}
}

void Valkyrie::ShowMenu()
{
	static std::string IconDev("menu-dev");
	static std::string IconSkinChanger("menu-cloth");
	static std::string IconSettings("menu-settings");

	if (InputController.IsDown(ShowMenuKey) && ImGui::Begin("Valkyrie", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize)) {

		/// Currently avg ping must be set manually since I didn't find how to get it from memory
		ImGui::SliderFloat("Average Ping", &AveragePing, 0.f, 150.f);
		ScriptContext.ping = AveragePing;

		ImGui::Image(GameData::GetImage(IconDev), ImVec2(15, 15));
		ImGui::SameLine();
		if (ImGui::BeginMenu("Development")) {
			DrawDevMenu();
			ImGui::EndMenu();
		}

		ImGui::Image(GameData::GetImage(IconSettings), ImVec2(15, 15));
		ImGui::SameLine();
		if (ImGui::BeginMenu("Menu Settings")) {
			DrawUIMenu();
			ImGui::EndMenu();
		}

		ImGui::Image(GameData::GetImage(IconSkinChanger), ImVec2(15, 15));
		ImGui::SameLine();
		if (ImGui::BeginMenu("Skin Changer")) {
			SkinChanger::ImGuiDraw();
			ImGui::EndMenu();
		}

		ImGui::Separator();
		ScriptManager.ImGuiDrawMenu(ScriptContext);

		ImGui::End();

		if (Configs.IsTimeToSave())
			SaveConfigs();
	}

	//ImGui::ShowDemoWindow();

	if (ShowConsoleWindow)
		ShowConsole();

	if (ShowObjectExplorerWindow)
		ObjectExplorer::ImGuiDraw(*CurrentGameState);

	if (ShowOffsetScanner)
		OffsetScanner::ImGuiDraw();
}

void Valkyrie::ShowConsole()
{
	ImGui::Begin("Console");
	
	int current = Logger::BufferStart;
	while (current != Logger::BufferEnd) {
		LogEntry& entry = Logger::Buffer[current];

		bool pushedColor = false;
		switch (entry.type) {
		
			case LOG_ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, Color::RED);
				pushedColor = true;
				ImGui::Text("[error]");
				ImGui::SameLine();
				break;
			case LOG_WARNING:
				ImGui::PushStyleColor(ImGuiCol_Text, Color::YELLOW);
				pushedColor = true;
				ImGui::Text("[warning]");
				ImGui::SameLine();
				break;
			case LOG_INFO:
				ImGui::Text("[info]");
				ImGui::SameLine();
				break;
		}

		ImGui::Text(entry.message);
		if (pushedColor)
			ImGui::PopStyleColor(1);

		current = Logger::NextIndex(current);
	}
	
	ImGui::End();
}

void Valkyrie::InitializeOverlay()
{
	Logger::Info("Initializing overlay");

	LeagueWindowHandle = FindWindowA("RiotWindowClass", NULL);
	OriginalWindowMessageHandler = WNDPROC(SetWindowLongA(LeagueWindowHandle, GWL_WNDPROC, LONG_PTR(HookedWindowMessageHandler)));

	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(LeagueWindowHandle))
		throw std::runtime_error("Failed to initialize ImGui_ImplWin32_Init");

	if (!ImGui_ImplDX9_Init(DxDevice))
		throw std::runtime_error("Failed to initialize ImGui_ImplDX9_Init");

	ImGui::GetIO().IniFilename = Globals::ImGuiIniPath.c_str();

	LoadConfigs();
}

void Valkyrie::InitializePython()
{
	Logger::Info("Initializing Python");
	PyImport_AppendInittab("valkyrie", &PyInit_valkyrie);
	Py_Initialize();
	exec("from valkyrie import *");
}

void Valkyrie::LoginAndLoadData()
{
	TaskPool.AddWorkers(1);

	char* name;
	char* pass;
	ValkyrieShared::LoadCredentials(&name, &pass);

	TaskPool.DispatchTask(
		"Logging In",
		Api.GetUser(IdentityInfo(name, pass, HardwareInfo::Calculate()), name),

		[](std::shared_ptr<AsyncTask> response) {
		LoggedUser = ((UserOperationAsync*)response.get())->user;
		TaskPool.DispatchTask(
			"Load Essentials",
			std::shared_ptr<GameDataEssentialsLoad>(new GameDataEssentialsLoad()),

			[](std::shared_ptr<AsyncTask> response) {
			EssentialsLoaded = true;
			TaskPool.DispatchTask(
				"Load Extras", std::shared_ptr<GameDataImagesLoad>(new GameDataImagesLoad()), [](std::shared_ptr<AsyncTask> response) {}
			);
		}
		);
	}
	);
}

void Valkyrie::LoadConfigs()
{
	auto configPath = Globals::ConfigsDir;
	configPath.append("valkyrie.cfg");

	Configs.SetConfigFile(configPath.u8string());
	Configs.Load();

	ShowConsoleWindow        = Configs.GetBool("show_console", false);
	ShowObjectExplorerWindow = Configs.GetBool("show_obj_explorer", false);
	ShowOffsetScanner        = Configs.GetBool("show_offset_scanner", false);

	ShowMenuKey              = (HKey)Configs.GetInt("show_key", HKey::Tab);
	MenuStyle                = SetStyle(Configs.GetInt("menu_style", 0));
	AveragePing              = Configs.GetFloat("ping", 60.0f);
}

void Valkyrie::SaveConfigs()
{
	Configs.SetBool("show_console", ShowConsoleWindow);
	Configs.SetBool("show_obj_explorer", ShowConsoleWindow);
	Configs.SetBool("show_offset_scanner", ShowOffsetScanner);
	Configs.SetInt("show_key", ShowMenuKey);
	Configs.SetInt("menu_style", MenuStyle);
	Configs.SetFloat("ping", AveragePing);
	Configs.Save();
}

void Valkyrie::LoadScripts()
{
	fs::path pathScripts = Globals::WorkingDir;
	pathScripts.append("scripts");
	std::string pathStr = pathScripts.u8string();

	ScriptManager.LoadScriptsFromFolder(pathStr);
}

void Valkyrie::ExecuteScripts()
{
	if(GetForegroundWindow() == LeagueWindowHandle)
		ScriptManager.ExecuteScripts(ScriptContext);
}

void Valkyrie::SetupScriptExecutionContext()
{
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("##Overlay", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoBackground
	);
	ScriptContext.SetGameState(CurrentGameState);
	ScriptContext.SetImGuiOverlay(ImGui::GetWindowDrawList());
	ImGui::End();
}

void Valkyrie::DrawDevMenu()
{
	if (ImGui::Button("Reload Scripts"))
		LoadScripts();

	ImGui::LabelText("VPath", Globals::WorkingDir.u8string().c_str());
	ImGui::LabelText("Offset Patch", Offsets::GameVersion.c_str());
	ImGui::Checkbox("Show Console",         &ShowConsoleWindow);
	ImGui::Checkbox("Show Object Explorer", &ShowObjectExplorerWindow);
	ImGui::Checkbox("Show Offset Scanner",  &ShowOffsetScanner);
	if (ImGui::BeginMenu("Core Benchmarks")) {
		Reader.GetBenchmarks().ImGuiDraw();
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Scripts Benchmarks")) {
		for (auto& script : ScriptManager.scripts) {
			ImGui::DragFloat(script->fileName.c_str(), &script->executionTimes[ScriptFunction::ON_LOOP].avgMs);
		}
		ImGui::EndMenu();
	}
}

void Valkyrie::DrawUIMenu()
{
	if (ChooseMenuStyle("Menu Style", MenuStyle))
		SetStyle(MenuStyle);
	ShowMenuKey = (HKey)InputController::ImGuiKeySelect("Show Menu Key", ShowMenuKey);
}

void Valkyrie::Update()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	__try {
		TaskPool.ImGuiDraw();
		if (EssentialsLoaded) {
			
			CurrentGameState = Reader.GetNextState();
			if (CurrentGameState->gameStarted) {
				SkinChanger::Refresh();
				SetupScriptExecutionContext();
				ShowMenu();
				ExecuteScripts();
			}
		}
	}
	__except (1) {
		Logger::Error("SEH exception occured in main loop. This shouldn't happen.");
	}

	ImGui::EndFrame();
	ImGui::Render();

	DxDeviceMutex.lock();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	DxDeviceMutex.unlock();
}

void Valkyrie::HookDirectX()
{
	static const int SearchLength               = 0x500000;
	static const int SetVertexShaderVTableIndex = 92;
	static const int PresentVTableIndex         = 17;
	static const int EndSceneVTableIndex        = 42;
	static const int SetTransformVTableIndex    = 44;

	Logger::Info("Hooking DirectX");

	DWORD objBase = (DWORD)LoadLibraryA("d3d9.dll");
	DWORD stopAt = objBase + SearchLength;

	Logger::Info("Found base of d3d9.dll at: %#010x", objBase);
	int skips = 0;
	while (objBase++ < stopAt)
	{
		if ((*(WORD*)(objBase + 0x00)) == 0x06C7
			&& (*(WORD*)(objBase + 0x06)) == 0x8689
			&& (*(WORD*)(objBase + 0x0C)) == 0x8689
			) {
			if (skips > 0) {
				skips--;
				continue;
			}
			objBase += 2;
			break;
		}
	}

	if (objBase >= stopAt)
		throw std::runtime_error("Did not find D3D device");
	Logger::Info("Found D3D Device at: %#010x", objBase);

	PDWORD VTable;
	*(DWORD*)& VTable = *(DWORD*)objBase;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OriginalD3DPresent = (D3DPresentFunc)(VTable[PresentVTableIndex]);
	LONG error = DetourAttach(&(PVOID&)OriginalD3DPresent, (PVOID)HookedD3DPresent);
	if (error)
		throw std::runtime_error(Strings::Format("Failed to hook DirectX Present. Detours error code: %d"));
	
	DetourTransactionCommit();
	
}

void Valkyrie::UnhookDirectX()
{
	Logger::Info("Unhooking DirectX");
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
		Logger::Error("Standard exception occured %s", error.what());
		UnhookDirectX();
	}
	catch (error_already_set&) {
		Logger::Error("Boost::Python exception occured %s", Script::GetPyError().c_str());
		UnhookDirectX();
	}
	catch (...) {
		Logger::Error("Unexpected exception occured");
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
		if (wParam > 31 && wParam < 127)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam > 31 && wParam < 127)
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
	ImGuiWindowMessageHandler(hWnd, msg, wParam, lParam);

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
