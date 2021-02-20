#pragma once
#include "ValkyrieAPI.h"
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

	void ShowRequestsStatus();

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayUserPanel();
	void DisplayAdminPanel();

	/// Flags for requests
	bool                                performUpdate = true;
	bool                                retrieveUsers = true;

	ValkyrieAPI                         api;
	DisplayMode                         displayMode = DM_LOGIN;

	HardwareInfo                        hardwareInfo;
	UserInfo                            loggedUser;
	std::vector<UserInfo>               allUsers;

	std::string trackIdInjector       = "Injector";
	std::string trackIdUpdate         = "Update";
	std::string trackIdLogin          = "LogIn";
	std::string trackIdGetUsers       = "GetUsers";
	std::string trackIdCreateAccount  = "CreateAccount";
	std::string trackIdCheckVersion   = "CheckVersion";
	std::string trackIdGenerateInvite = "GenerateInvite";

	const std::string DLL_PATH_PYTHON   = "\\boost_python39-vc141-mt-x32-1_75.dll";
	const std::string DLL_PATH_VALKYRIE = "\\ValkyrieDLL.dll";

	/// Login stuff
	static const int INPUT_TEXT_BUFF_SIZE            = 256;
	char             nameBuff[INPUT_TEXT_BUFF_SIZE]  = "your_username";
	char             passBuff[INPUT_TEXT_BUFF_SIZE]  = "<pass>";

	/// Create account stuff
	char             passConfirmBuff[INPUT_TEXT_BUFF_SIZE] = "<pass>";
	char             discordBuff[INPUT_TEXT_BUFF_SIZE]     = "your_discord#0000";
	char             inviteCodeBuff[INPUT_TEXT_BUFF_SIZE]  = "your_invite_code";

	/// Invite code generator stuff
	float            inviteSubscriptionDays;
	char             generatedInviteCodeBuff[INPUT_TEXT_BUFF_SIZE] = "";

	/// User Manager stuff
	int              selectedUser = 0;
	float            deltaDays = 0.f;

	AsyncTaskPool    taskPool;
};

class UpdaterAsync : public AsyncTask {

public:

	UpdaterAsync(ValkyrieLoader& vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	ValkyrieLoader&                   loader;
	std::shared_ptr<GetS3ObjectAsync> updateFile;

	virtual void Perform();
};