#include "Valkyrie.h"
#include "Strings.h"
#include "detours.h"
#include "GameData.h"
#include "ObjectExplorer.h"
#include "Offsets.h"
#include "Memory.h"
#include "Paths.h"
#include "PyStructs.h"
#include "OffsetScanner.h"
#include "ValkyrieShared.h"
#include "GameKeybind.h"
#include "Console.h"

#include "FakeMouse.h"
#include "D3DX9Shader.h"


#include <boost/exception/diagnostic_information.hpp>

#include <stdexcept>
#include <iostream>
#include <functional>
#include <dxva2api.h>
#include <TlHelp32.h>

#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))

D3DPresentFunc                     Valkyrie::OriginalD3DPresent           = NULL;
WNDPROC                            Valkyrie::OriginalWindowMessageHandler = NULL;
LPDIRECT3DDEVICE9                  Valkyrie::DxDevice                     = NULL;
std::mutex                         Valkyrie::DxDeviceMutex;
HWND                               Valkyrie::LeagueWindowHandle;
RECT                               Valkyrie::WindowRect;

ValkyrieAPI*                       Valkyrie::Api = ValkyrieAPI::Get();
UserInfo                           Valkyrie::LoggedUser;
AsyncTaskPool*                     Valkyrie::TaskPool = AsyncTaskPool::Get();
bool                               Valkyrie::LoadedScripts = false;

GameReader                         Valkyrie::Reader;
PyExecutionContext                 Valkyrie::ScriptContext;
ScriptManager                      Valkyrie::ScriptManager;
GameState*                         Valkyrie::CurrentGameState = NULL;

InputController                    Valkyrie::InputController;
ConfigSet                          Valkyrie::Configs;
bool                               Valkyrie::ShowConsoleWindow;
bool                               Valkyrie::ShowObjectExplorerWindow;
bool                               Valkyrie::ShowOffsetScanner;
bool                               Valkyrie::ShowBenchmarkWindow;
HKey                               Valkyrie::ShowMenuKey;
bool                               Valkyrie::ShowMenuKeyShouldHold = true;
int                                Valkyrie::MenuStyle;
float                              Valkyrie::AveragePing;
HMODULE                            Valkyrie::ValkyrieDLLHandle;

bool ChooseMenuStyle(const char* label, int& currentStyle)
{
	return ImGui::Combo(label, &currentStyle, "Dark\0Light\0Classic\0Valkyrie\0");
}

int SetStyle(int style) {
	switch (style)
	{
	case 0: ImGui::StyleColorsDark(); break;
	case 1: ImGui::StyleColorsLight(); break;
	case 2: ImGui::StyleColorsClassic(); break;
	case 3: ValkyrieShared::ImGuiStyleValkyrie(); break;
	}
	return style;
}

void Valkyrie::Run()
{
	try {
		DxDeviceMutex.lock();

		GameKeybind::InitFromGameConfigs();
		DirectInputHook::Hook();
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
	DBG_INFO("Valkyrie::ShowMenu")
	static bool IsMenuToggled = false;

	if (ShowMenuKeyShouldHold) {
		if (InputController.IsDown(ShowMenuKey))
			DrawSettings();
	}
	else {
		if (InputController.WasPressed(ShowMenuKey))
			IsMenuToggled = !IsMenuToggled;

		if (IsMenuToggled)
			DrawSettings();
	}

	if (InputController.WasPressed(HKey::F3)) {
		ShowConsoleWindow = !ShowConsoleWindow;
	}

	if (ShowConsoleWindow) {
		static Console console;

		console.ImDraw(ScriptContext);
		ShowConsole();
	}

	if (ShowObjectExplorerWindow)
		ObjectExplorer::ImGuiDraw(*CurrentGameState);

	if (ShowOffsetScanner)
		OffsetScanner::ImGuiDraw();

	if (ShowBenchmarkWindow)
		DrawBenchmarkWindow();
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
	OriginalWindowMessageHandler = (WNDPROC)SetWindowLongPtr(LeagueWindowHandle, GWL_WNDPROC, (LONG)HookedWindowMessageHandler);

	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(LeagueWindowHandle))
		throw std::runtime_error("Failed to initialize ImGui_ImplWin32_Init");

	if (!ImGui_ImplDX9_Init(DxDevice))
		throw std::runtime_error("Failed to initialize ImGui_ImplDX9_Init");

	ImGui::GetIO().IniFilename = Paths::ImguiConfig.c_str();
	ValkyrieShared::ImGuiSetupSizesAndFont();
	LoadConfigs();
	GetWindowRect(LeagueWindowHandle, &WindowRect);
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
	TaskPool->AddWorkers(1);
	
	Logger::Info("Checking hardware");
	auto hardwareInfo = HardwareInfo::Calculate();
	
	Logger::Info("Logging in");
	char* name;
	char* pass;
	ValkyrieShared::LoadCredentials(&name, &pass);

	TaskPool->DispatchTask(
		"Logging In",
		Api->GetUser(IdentityInfo(name, pass, hardwareInfo), name),

		[](std::shared_ptr<AsyncTask> response) {
		LoggedUser = ((UserResultAsync*)response.get())->user;
		GameData::LoadEverything();
	}
	);
}

void Valkyrie::LoadConfigs()
{
	auto configPath = Paths::Configs;
	configPath.append("\\");
	configPath.append("valkyrie.cfg");

	Configs.SetConfigFile(configPath);
	Configs.Load();

	ShowConsoleWindow        = Configs.GetBool("show_console", false);
	ShowObjectExplorerWindow = Configs.GetBool("show_obj_explorer", false);
	ShowOffsetScanner        = Configs.GetBool("show_offset_scanner", false);
	ShowBenchmarkWindow      = Configs.GetBool("show_benchmarks", false);

	ShowMenuKey              = (HKey)Configs.GetInt("show_key", HKey::Tab);
	ShowMenuKeyShouldHold    = Configs.GetBool("show_key_hold", ShowMenuKeyShouldHold);
	MenuStyle                = SetStyle(Configs.GetInt("menu_style", 3));
	AveragePing              = Configs.GetFloat("ping", 60.0f);
}

void Valkyrie::SaveConfigs()
{
	Configs.SetBool("show_benchmarks", ShowBenchmarkWindow);
	Configs.SetBool("show_console", ShowConsoleWindow);
	Configs.SetBool("show_obj_explorer", ShowConsoleWindow);
	Configs.SetBool("show_offset_scanner", ShowOffsetScanner);

	Configs.SetBool("show_key_hold", ShowMenuKeyShouldHold);
	Configs.SetInt("show_key", ShowMenuKey);
	Configs.SetInt("menu_style", MenuStyle);
	Configs.SetFloat("ping", AveragePing);
	Configs.Save();
}

void Valkyrie::ExecuteScripts()
{
	DBG_INFO("Executing scripts")
	if (!LoadedScripts) {
		ScriptManager.LoadAllScripts(CurrentGameState);
		LoadedScripts = true;
	}
	ScriptManager.ExecuteScripts(ScriptContext);
}

void Valkyrie::SetupScripts()
{
	DBG_INFO("Setting up scripts")
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

void Valkyrie::DrawSettings()
{
	static std::string IconDev("menu-dev");
	static std::string IconSettings("menu-settings");

	ImGui::Begin("Valkyrie", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

	/// Currently avg ping must be set manually since I didn't find how to get it from memory
	ImGui::SliderFloat("Average Ping", &AveragePing, 0.f, 150.f);
	ScriptContext.ping = AveragePing;

	ImGui::Image(GameData::GetImage(IconDev), ImVec2(15, 15));
	ImGui::SameLine();
	if (ImGui::BeginMenu("Development")) {
		DrawDevelopmentSettings();
		ImGui::EndMenu();
	}

	ImGui::Image(GameData::GetImage(IconSettings), ImVec2(15, 15));
	ImGui::SameLine();
	if (ImGui::BeginMenu("Menu Settings")) {
		DrawMenuSettings();
		ImGui::EndMenu();
	}

	ImGui::Separator();
	ScriptManager.ImGuiDrawMenu(ScriptContext);

	ImGui::End();

	if (Configs.IsTimeToSave())
		SaveConfigs();
}

void Valkyrie::DrawDevelopmentSettings()
{
	if (ImGui::Button("Reload Scripts"))
		LoadedScripts = false;

	ImGui::SameLine();
	if (ImGui::Button("Reload Essentials")) {
		GameKeybind::InitFromGameConfigs();
		GameData::LoadEssentials();
	}

	ImGui::LabelText("Offset Patch", Offsets::GameVersion.c_str());
	ImGui::Checkbox("Show Console",         &ShowConsoleWindow);
	ImGui::Checkbox("Show Object Explorer", &ShowObjectExplorerWindow);
	ImGui::Checkbox("Show Offset Scanner",  &ShowOffsetScanner);
	ImGui::Checkbox("Show Benchmarks",      &ShowBenchmarkWindow);
}

void Valkyrie::DrawMenuSettings()
{
	if (ChooseMenuStyle("Menu Style", MenuStyle))
		SetStyle(MenuStyle);
	ShowMenuKey = (HKey)InputController::ImGuiKeySelect("Show Menu Key", ShowMenuKey);
	ImGui::Checkbox("Must Hold Menu Key", &ShowMenuKeyShouldHold);
}

void Valkyrie::DrawBenchmarkWindow()
{
	ImGui::Begin("Benchmarks");
	ImGui::BeginTabBar("BenchmarkTabs");

	if (ImGui::BeginTabItem("Core Benchmarks")) {
		Reader.GetBenchmarks().ImGuiDraw();
		ImGui::DragFloat("Collision Engine", &ScriptContext.collisionEngine.updateTimeMs.avgMs);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Core Scripts")) {
		for (auto& script : ScriptManager.coreScripts) {
			ImGui::DragFloat(script->info->id.c_str(), &script->executionTimes[ScriptFunction::ON_LOOP].avgMs);
		}
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Community Scripts")) {
		for (auto& script : ScriptManager.communityScripts) {
			ImGui::DragFloat(script->info->id.c_str(), &script->executionTimes[ScriptFunction::ON_LOOP].avgMs);
		}
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
}

void Valkyrie::Update()
{
	DBG_INFO("Valkyrie::Update")
	__try {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		TaskPool->ImGuiDraw();
		if (GameData::EssentialsLoaded && GetForegroundWindow() == LeagueWindowHandle) {
			
			CurrentGameState = Reader.GetNextState();
			if (CurrentGameState->gameStarted) {
				SetupScripts();
				ShowMenu();
				ExecuteScripts();
			}
		}

		ImGui::EndFrame();
		ImGui::Render();

		/// Render
		DxDeviceMutex.lock();
		if (GameData::EverythingLoaded && CurrentGameState != nullptr)
			CurrentGameState->renderer.DrawOverlay(DxDevice);
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DxDeviceMutex.unlock();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Logger::Error("SEH exception occured in main loop.");
		DBG_DUMP()
	}
	DBG_CLEAR()
}

void Valkyrie::HookDirectX()
{
	static const int SearchLength               = 0x500000;
	static const int SetVertexShaderVTableIndex = 92;
	static const int PresentVTableIndex         = 17;
	static const int EndSceneVTableIndex        = 42;
	static const int SetTransformVTableIndex    = 44;

	Logger::Info("Hooking DirectX");

	HWND window = FindWindowA("RiotWindowClass", NULL);
	IDirect3D9 * pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		throw std::runtime_error("Failed to get direct3d");

	D3DPRESENT_PARAMETERS d3dpp{ 0 };
	d3dpp.hDeviceWindow = window, d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD, d3dpp.Windowed = TRUE;

	IDirect3DDevice9 *device = nullptr;
	if (FAILED(pD3D->CreateDevice(0, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device)))
	{
		pD3D->Release();
		throw std::runtime_error("Failed to create dx device");
	}

	void ** VTable = *reinterpret_cast<void***>(device);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OriginalD3DPresent = (D3DPresentFunc)(VTable[PresentVTableIndex]);
	LONG error = DetourAttach(&(PVOID&)OriginalD3DPresent, (PVOID)HookedD3DPresent);
	if (error)
		throw std::runtime_error(Strings::Format("DetourAttach: Failed to hook DirectX Present. Detours error code: %d", error));
	
	error = DetourTransactionCommit();
	if (error)
		throw std::runtime_error(Strings::Format("DetourCommitTransaction: Failed to hook DirectX Present. Detours error code: %d", error));

	device->Release();
	Logger::Info("Successfully hooked DirectX");
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
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam != 9)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam != 9)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}

	return true;
}

LRESULT WINAPI Valkyrie::HookedWindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiWindowMessageHandler(hWnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_MOVE:
		GetWindowRect(Valkyrie::LeagueWindowHandle, &Valkyrie::WindowRect);
		return 0;
	case WM_SIZE:
		return 0;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return CallWindowProcA(OriginalWindowMessageHandler, hWnd, msg, wParam, lParam);
}
