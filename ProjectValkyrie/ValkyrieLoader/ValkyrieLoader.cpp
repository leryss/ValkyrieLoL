#include "ValkyrieLoader.h"
#include "AsyncInjector.h"
#include "AsyncCheatUpdater.h"
#include "AsyncLoaderUpdater.h"
#include "ValkyrieShared.h"
#include "Strings.h"
#include "Paths.h"

#include "imgui/imgui.h"
#include "miniz/miniz.h"

#include <fstream>
#include <windows.h>

ValkyrieLoader::ValkyrieLoader()
{
	taskPool->AddWorkers(4);

	SetupWorkingDir();
	hardwareInfo = HardwareInfo::Calculate();
	LoadConfigs();

	currentPanel = &loginPanel;
}

void ValkyrieLoader::ImGuiShow()
{
	if (performLoaderUpdate) {

		UpdateLoader();
		performLoaderUpdate = false;
	}

	taskPool->ImGuiDraw();

	if (loaderUpdated) {
		if (configs.IsTimeToSave())
			SaveConfigs();

		if (currentPanel != nullptr)
			currentPanel->Draw(*this);

		if (loggedUser.level >= USER_LEVEL_ADMIN) {
			adminPanel.Draw(*this);
		}
	}
}

void ValkyrieLoader::LoadConfigs()
{
	configs.SetSaveInterval(5000);
	configs.SetConfigFile("vload.cfg");
	configs.Load();

	loaderVersionHash = configs.GetStr("loader_version", "");
	cheatVersionHash = configs.GetStr("cheat_version", "");
	loginPanel.autoSaveCredentials  = loginPanel.loadCredentials = configs.GetBool("save_credentials", false);
	userPanel.autoInject = configs.GetBool("auto_inject", false);
}

void ValkyrieLoader::SaveConfigs()
{
	configs.SetBool("save_credentials", loginPanel.autoSaveCredentials);
	configs.SetBool("auto_inject", userPanel.autoInject);
	configs.SetStr("loader_version", loaderVersionHash.c_str());
	configs.SetStr("cheat_version", cheatVersionHash.c_str());
	configs.Save();
}

void ValkyrieLoader::SetupWorkingDir()
{
	/// Check if valkyrie dir exists
	const char* path = Paths::Root.c_str();

	bool directoryExists = false;
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY)
		directoryExists = true;

	/// Create valkyrie dir if not exists
	if (!directoryExists) {
		if (!CreateDirectoryA(path, NULL)) {
			throw std::exception("Fatal error. Couldn't create valkyrie folder");
		}
	}
}

void ValkyrieLoader::UpdateLoader()
{
	taskPool->DispatchTask(
		std::string("LoaderCheckUpdate"),
		api->GetCheatS3Object("valkyrie-releases-eu-north-1", "loader.exe"),

		[this](std::shared_ptr<AsyncTask> response) {
			taskPool->DispatchTask(
				std::string("LoaderUpdate"),
				std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncLoaderUpdater(*this, std::static_pointer_cast<GetS3ObjectAsync>(response))),
				[this](std::shared_ptr<AsyncTask> response) {

					loaderUpdated = true;
				}
			);
		}
	);
}
