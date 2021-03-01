#pragma once
#include <vector>
#include <map>
#include <chrono>
#include <cstdlib>

#include "ValkyrieAPI.h"
#include "ConfigSet.h"
#include "AsyncTaskPool.h"

#include "UserPanel.h"
#include "LoginPanel.h"
#include "CreateAccountPanel.h"
#include "AdminPanel.h"

using namespace std::chrono;

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_USER_PANEL
};

class ValkyrieLoader {

public:
	ValkyrieLoader();
	void ImGuiShow();

	UserPanel          userPanel;
	AdminPanel         adminPanel;
	LoginPanel         loginPanel;
	CreateAccountPanel createAccPanel;
	LoaderPanel*       currentPanel;
				       
	UserInfo           loggedUser;
	IdentityInfo       identity;
	HardwareInfo       hardwareInfo;
				       
	std::string        versionFilePath;
	std::string        versionHash;
				       
	ConfigSet          configs;

private:

	void LoadConfigs();
	void SaveConfigs();
	void SetupWorkingDir();
	void ReadVersion();

	AsyncTaskPool* taskPool = AsyncTaskPool::Get();
};