#pragma once
#include "ValkyrieAPI.h"
#include "ConfigSet.h"
#include <vector>
#include <map>

#include "AsyncTaskPool.h"

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_USER_PANEL
};

#define MB_IN_BYTES 1000000

class ValkyrieLoader {

public:
	ValkyrieLoader();
	void ImGuiShow();

	std::string                         changeLog;
	std::string                         valkyrieFolder;
	std::string                         versionFilePath;
	std::string                         versionHash;

private:

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayUserPanel();
	void DisplayAdminPanel();

	void DrawUserManager();
	void DrawUserManagerFilter();
	void DrawUserManagerActions();
	void DrawInviteGenerator();

	void LoadConfigs();
	void SaveConfigs();
	void SetupWorkingDir();
	void ReadVersion();

	void RetrieveUsersIfNecessarry();
	void SortUsersIfNecessarry();

	/// Flags for requests
	bool                                performUpdate = true;
	bool                                retrieveUsers = true;
	
	ConfigSet                           configs;
	bool                                autoInject;
	bool                                autoSaveCredentials;

	std::shared_ptr<AsyncTask>          injectorTask;
	bool                                updateComplete = false;
	bool                                loadCredentials;

	ValkyrieAPI                         api;
	DisplayMode                         displayMode = DM_LOGIN;

	HardwareInfo                        hardwareInfo;
	UserInfo                            loggedUser;
	std::vector<UserInfo>               allUsers;

	std::string trackIdInjector       = "Injector";
	std::string trackIdUpdate         = "Update";
	std::string trackIdUpdateUser     = "UpdateUser";
	std::string trackIdLogin          = "LogIn";
	std::string trackIdGetUsers       = "GetUsers";
	std::string trackIdCreateAccount  = "CreateAccount";
	std::string trackIdCheckVersion   = "CheckVersion";
	std::string trackIdGenerateInvite = "GenerateInvite";

	const std::string DLL_PATH_VALKYRIE = "\\payload\\ValkyrieDLL.dll";

	/// Login stuff
	static const int INPUT_TEXT_BUFF_SIZE            = 256;
	char             nameBuff[INPUT_TEXT_BUFF_SIZE]  = "your_username";
	char             passBuff[INPUT_TEXT_BUFF_SIZE]  = "<pass>";

	/// Create account stuff
	char             passConfirmBuff[INPUT_TEXT_BUFF_SIZE] = "<pass>";
	char             discordBuff[INPUT_TEXT_BUFF_SIZE]     = "your_discord#0000";
	char             inviteCodeBuff[INPUT_TEXT_BUFF_SIZE]  = "your_invite_code";

	/// Invite code generator stuff
	int              inviteRole = 0;
	float            inviteSubscriptionDays = 30.f;
	char             generatedInviteCodeBuff[INPUT_TEXT_BUFF_SIZE] = "";

	/// User Manager stuff
	int              selectedRole = 0;
	int              selectedUser = 0;
	float            deltaDays = 2.f;
	char             userFilter[INPUT_TEXT_BUFF_SIZE];
	std::vector<bool>filterMask;

	AsyncTaskPool    taskPool;
};