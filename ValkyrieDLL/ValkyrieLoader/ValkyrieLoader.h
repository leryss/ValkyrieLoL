#pragma once
#include "ValkyrieAPI.h"
#include <vector>
#include <map>

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_PANEL
};

enum UpdateState {
	US_NOT_STARTED,
	US_DOWNLOADING,
	US_EXTRACTING,
	US_FAILED,
	US_COMPLETE
};

class AsyncRequest {
public:
	bool triggeredCallbacks = false;

	std::shared_ptr<BaseAPIResponse>                                response;
	std::function<void(std::shared_ptr<BaseAPIResponse>& response)> onSuccess;
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
	void UpdateValkyrie(Aws::IOStream& updateFileStream);

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayPanel();

	bool                                performUpdate = true;

	std::string                         valkyrieFolder;
	std::string                         versionFilePath;
	std::string                         versionHash;
	std::shared_ptr<UpdaterProgress>    updater;

	Aws::String                         apiToken;
	ValkyrieAPI                         api;
	DisplayMode                         displayMode = DM_LOGIN;

	/// Login stuff
	static const int INPUT_TEXT_BUFF_SIZE            = 256;
	char             nameBuff[INPUT_TEXT_BUFF_SIZE]  = "your_username";
	char             passBuff[INPUT_TEXT_BUFF_SIZE]  = "<pass>";

	/// Create account stuff
	char             passConfirmBuff[INPUT_TEXT_BUFF_SIZE] = "<pass>";
	char             discordBuff[INPUT_TEXT_BUFF_SIZE]     = "your_discord#0000";
	char             inviteCodeBuff[INPUT_TEXT_BUFF_SIZE]  = "your_invite_code";

	std::map<std::string, std::shared_ptr<AsyncRequest>> asyncRequests;

};