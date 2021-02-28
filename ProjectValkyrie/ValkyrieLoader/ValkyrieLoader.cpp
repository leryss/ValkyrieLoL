#include "ValkyrieLoader.h"
#include "AsyncInjector.h"
#include "AsyncUpdater.h"
#include "ValkyrieShared.h"
#include "Strings.h"

#include "imgui/imgui.h"
#include "miniz/miniz.h"

#include <fstream>
#include <windows.h>

ValkyrieLoader::ValkyrieLoader()
{
	taskPool->AddWorkers(4);

	SetupWorkingDir();
	ReadVersion();
	hardwareInfo = HardwareInfo::Calculate();
	LoadConfigs();

	currentPanel = &loginPanel;
}

void ValkyrieLoader::ImGuiShow()
{
	auto& io = ImGui::GetIO();

	if(currentPanel != nullptr)
		currentPanel->Draw(*this);

	if (loggedUser.level >= USER_LEVEL_ADMIN) {
		adminPanel.Draw(*this);
	}

	taskPool->ImGuiDraw();
	if (configs.IsTimeToSave())
		SaveConfigs();
}

std::string ValkyrieLoader::GetDllPath()
{
	return valkyrieFolder + "\\payload\\ValkyrieDLL.dll";
}

std::string ValkyrieLoader::GetDependenciesPath()
{
	return valkyrieFolder + "\\dependencies";
}

std::string ValkyrieLoader::GetChangeLogPath()
{
	return valkyrieFolder + "\\changelog.txt";
}

void ValkyrieLoader::LoadConfigs()
{
	configs.SetSaveInterval(5000);
	configs.SetConfigFile("vload.cfg");
	configs.Load();

	loginPanel.autoSaveCredentials  = loginPanel.loadCredentials = configs.GetBool("save_credentials", false);
	userPanel.autoInject = configs.GetBool("auto_inject", false);
}

void ValkyrieLoader::SaveConfigs()
{
	configs.SetBool("save_credentials", loginPanel.autoSaveCredentials);
	configs.SetBool("auto_inject", userPanel.autoInject);

	configs.Save();
}

void ValkyrieLoader::SetupWorkingDir()
{
	valkyrieFolder = ValkyrieShared::GetWorkingDir();

	/// Check if valkyrie dir exists
	bool directoryExists = false;
	DWORD ftyp = GetFileAttributesA(valkyrieFolder.c_str());
	if (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY)
		directoryExists = true;

	/// Create valkyrie dir if not exists
	if (!directoryExists) {
		if (!CreateDirectoryA(valkyrieFolder.c_str(), NULL)) {
			throw std::exception("Fatal error. Couldn't create valkyrie folder");
		}
	}
}

void ValkyrieLoader::ReadVersion()
{
	versionFilePath = valkyrieFolder + "\\version";
	DWORD ftyp = GetFileAttributesA(versionFilePath.c_str());
	if (ftyp != INVALID_FILE_ATTRIBUTES) {
		std::ifstream versionFile(versionFilePath);
		std::getline(versionFile, versionHash);
	}
}