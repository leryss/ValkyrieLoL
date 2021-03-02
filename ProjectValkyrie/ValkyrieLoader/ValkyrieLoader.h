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
	void               ImGuiShow();
	void               SaveConfigs();

	UserPanel          userPanel;
	AdminPanel         adminPanel;
	LoginPanel         loginPanel;
	CreateAccountPanel createAccPanel;
	LoaderPanel*       currentPanel;
				       
	UserInfo           loggedUser;
	IdentityInfo       identity;
	HardwareInfo       hardwareInfo;
				       
	std::string        cheatVersionHash;
	std::string        loaderVersionHash;
				       
	ConfigSet          configs;

private:

	bool performLoaderUpdate = true;
	bool loaderUpdated = false;

	void LoadConfigs();
	void SetupWorkingDir();
	void UpdateLoader();

	AsyncTaskPool* taskPool = AsyncTaskPool::Get();
	ValkyrieAPI* api        = ValkyrieAPI::Get();
};