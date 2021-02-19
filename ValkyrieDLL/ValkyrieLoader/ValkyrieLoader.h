#pragma once
#include "ValkyrieAPI.h"
#include <vector>
#include <map>

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_USER_PANEL
};

enum UpdateState {
	US_NOT_STARTED,
	US_DOWNLOADING,
	US_EXTRACTING,
	US_FAILED,
	US_COMPLETE
};

class AsyncRequestTracker {
public:
	std::shared_ptr<APIAsyncRequest>                                request;
	std::function<void(std::shared_ptr<APIAsyncRequest>& response)> onSuccess;
};

class UpdaterProgress {

public:
	UpdaterProgress(Aws::IOStream& stream) {
		stream.seekg(0, stream.end);
		this->sizeDownloadBuff = stream.tellg();
		stream.seekg(0, stream.beg);

		this->downloadBuff = new char[sizeDownloadBuff];
		this->stream = &stream;
	}

	UpdaterProgress() {
		delete[] downloadBuff;
	}

	
	UpdateState    status = US_NOT_STARTED;
	std::string    error;

	int            numFilesToExtract;
	int            numFilesExtracted;

	Aws::IOStream* stream;
	int            sizeDownload   = 0;
	int            sizeDownloadBuff;
	char*          downloadBuff;
};

class ValkyrieLoader {

public:
	ValkyrieLoader();
	void ImGuiShow();

private:

	void ShowRequestsStatus();
	void ShowUpdateStatus();
	void UpdateValkyrie(std::shared_ptr<GetS3ObjectAsync> updateResponse);

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayUserPanel();
	void DisplayAdminPanel();

	bool TaskNotExecuting(std::string trackingId);
	void TrackRequest(std::string trackingId, std::shared_ptr<APIAsyncRequest> request, std::function<void(std::shared_ptr<APIAsyncRequest>& response)> onSuccess);

	/// Flags for requests
	bool                                performUpdate = true;
	bool                                retrieveUsers = true;

	std::string                         changeLog;
	std::string                         valkyrieFolder;
	std::string                         versionFilePath;
	std::string                         versionHash;
	std::shared_ptr<UpdaterProgress>    updater;

	ValkyrieAPI                         api;
	DisplayMode                         displayMode = DM_LOGIN;

	HardwareInfo                        hardwareInfo;
	UserInfo                            loggedUser;
	std::vector<UserInfo>               allUsers;

	std::string trackIdLogin          = "LogIn";
	std::string trackIdGetUsers       = "GetUsers";
	std::string trackIdCreateAccount  = "CreateAccount";
	std::string trackIdCheckVersion   = "CheckVersion";
	std::string trackIdGenerateInvite = "GenerateInvite";

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

	std::map<std::string, std::shared_ptr<AsyncRequestTracker>> asyncRequests;

};