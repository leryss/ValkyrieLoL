#include "ValkyrieLoader.h"

#include "imgui/imgui.h"
#include "miniz/miniz.h"

#include <fstream>
#include <windows.h>
#include <shlobj_core.h>
#include <chrono>

using namespace std::chrono;

static ImVec4      COLOR_RED    = ImVec4(1.f, 0.f, 0.f, 1.f);
static ImVec4      COLOR_PURPLE = ImVec4(0.5f, 0.3f, 0.8f, 1.f);
static ImVec4      COLOR_GREEN  = ImVec4(0.f, 1.f, 0.f, 1.f);
static ImVec4      COLOR_YELLOW = ImVec4(1.f, 1.f, 0.f, 1.f);

static const int   MB_IN_BYTES  = 1000000;
static const float ONE_DAY_SECS = 60.f * 60.f * 24.f;

ValkyrieLoader::ValkyrieLoader()
{
	/// Get appdata folder and create path for valkyrie folder
	char path[1024];
	if (!SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, TRUE))
		throw std::exception("Fatal error. Couldn't get appdata folder.");

	valkyrieFolder = std::string(path) + "\\" + "Valkyrie";

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

	/// Get version hash from file for auto update checking
	versionFilePath = valkyrieFolder + "\\version";
	ftyp = GetFileAttributesA(versionFilePath.c_str());
	if (ftyp != INVALID_FILE_ATTRIBUTES) {
		std::ifstream versionFile(versionFilePath);
		std::getline(versionFile, versionHash);
	}

	/// Calculate hardware info for HWID
	hardwareInfo = HardwareInfo::Calculate();
	printf(hardwareInfo.ToJsonString().c_str());
}

void ValkyrieLoader::ImGuiShow()
{
	auto& io = ImGui::GetIO();

	switch (displayMode) {
	case DM_LOGIN:
		DisplayLogin();
		break;
	case DM_USER_PANEL:
		DisplayUserPanel();
		break;
	case DM_CREATE_ACCOUNT:
		DisplayCreateAccount();
		break;
	}

	if (loggedUser.level > 0) {
		DisplayAdminPanel();
	}
}

void ValkyrieLoader::ShowRequestsStatus()
{
	DWORD tickCount = GetTickCount() % 600;

	auto it = asyncRequests.begin();
	while (it != asyncRequests.end()) {
		auto& pair = *it;
		auto  operationName = pair.first.c_str();
		auto& req = pair.second;

		switch (req->response->status) {
		case RS_EXECUTING:
			ImGui::Separator();
			ImGui::TextColored(COLOR_PURPLE, "Performing: %s%s", operationName, (tickCount < 200 ? "." : (tickCount < 400 ? ".." : "...")));
			break;
		case RS_FAILURE:
			ImGui::Separator();
			ImGui::TextColored(COLOR_RED, "Failed `%s`: %s", operationName, req->response->error.c_str());
			break;
		case RS_SUCCESS:
			if (!req->triggeredCallbacks) {
				req->onSuccess(req->response);
				req->triggeredCallbacks = true;
			}
			break;
		}

		++it;
	}
}

void ValkyrieLoader::ShowUpdateStatus()
{
	if (updater == nullptr)
		return;

	switch (updater->status) {
	case US_COMPLETE:
		ImGui::Separator();
		ImGui::TextColored(COLOR_GREEN, "Valkyrie is up to date with the latest version.");
		break;
	case US_FAILED:
		ImGui::Separator();
		ImGui::TextColored(COLOR_RED, "Updating failed: %s", updater->error);
		break;
	case US_EXTRACTING:
		ImGui::Separator();
		ImGui::Text("Unpacking updates %d / %d", updater->numFilesExtracted, updater->numFilesToExtract);
		break;
	case US_DOWNLOADING:
		ImGui::Separator();
		ImGui::Text("Downloading updates: %.2f / %.2f MB", float(updater->sizeDownload) / MB_IN_BYTES, float(updater->sizeDownloadBuff) / MB_IN_BYTES);
		break;
	}
}

void ValkyrieLoader::UpdateValkyrie(GetS3ObjectResponse* updateResponse)
{
	auto& updateFileStream = updateResponse->result.GetBody();
	updater = std::shared_ptr<UpdaterProgress>(new UpdaterProgress(updateFileStream));
	
	/// Check if there is a new version and download the files
	auto& etag = updateResponse->result.GetETag();
	if (etag.compare(versionHash.c_str()) != 0) {
		
		/// Download from stream
		updater->status = US_DOWNLOADING;

		while (true) {
			std::streamsize readBytes = updateFileStream.readsome(updater->downloadBuff + updater->sizeDownload, 10000);
			updater->sizeDownload += readBytes;
			if (readBytes == 0)
				break;
		}

		/// Extract from downloaded archive
		updater->status = US_EXTRACTING;

		mz_zip_archive archive;
		memset(&archive, 0, sizeof(archive));
		if (!mz_zip_reader_init_mem(&archive, updater->downloadBuff, updater->sizeDownload, 0)) {
			updater->status = US_FAILED;
			updater->error = "Failed to open update archive";
		}

		updater->numFilesToExtract = mz_zip_reader_get_num_files(&archive);
		for (int i = 0; i < updater->numFilesToExtract; ++i) {
			mz_zip_archive_file_stat fileStat;
			if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
				updater->status = US_FAILED;
				updater->error = "Failed to get archived file info for";
				return;
			}

			std::string path = valkyrieFolder + "\\" + fileStat.m_filename;
			if (fileStat.m_is_directory) {
				CreateDirectoryA(path.c_str(), NULL);
			}
			else if (!mz_zip_reader_extract_file_to_file(&archive, fileStat.m_filename, path.c_str(), 0)) {
				updater->status = US_FAILED;
				updater->error = std::string("Failed to unzip file ") + fileStat.m_filename;
				return;
			}

			updater->numFilesExtracted = i;
		}

		mz_zip_reader_end(&archive);

		/// Update version hash
		versionHash = etag.c_str();
		std::ofstream versionFile(valkyrieFolder + "\\version");
		versionFile.write(versionHash.c_str(), versionHash.size());
	}

	/// Read change logs
	std::ifstream changeLogFile(valkyrieFolder + "\\changelog.txt");
	changeLog = std::string((std::istreambuf_iterator<char>(changeLogFile)), std::istreambuf_iterator<char>());

	updater->status = US_COMPLETE;
}

void ValkyrieLoader::DisplayLogin()
{
	ImGui::Begin("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Name", nameBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Password", passBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);

	ImGui::Separator();
	if (ImGui::Button("Login")) {
		std::shared_ptr<AsyncRequest> request(new AsyncRequest());
		request->response      = api.GetUser(IdentityInfo(nameBuff, passBuff, hardwareInfo), nameBuff);
		request->onSuccess     = [this](std::shared_ptr<BaseAPIResponse>& response) {
			loggedUser = ((GetUserInfoResponse*)response.get())->user;
			displayMode    = DM_USER_PANEL;
		};
		asyncRequests["logging in"] = request;
	}
	ImGui::SameLine();
	if (ImGui::Button("Use invite code"))
		displayMode = DM_CREATE_ACCOUNT;

	ShowRequestsStatus();
	ImGui::End();
}

void ValkyrieLoader::DisplayCreateAccount()
{
	ImGui::Begin("Create account", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Invite Code",      inviteCodeBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Name",             nameBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Password",         passBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);
	ImGui::InputText("Confirm Password", passConfirmBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);
	ImGui::InputText("Discord",          discordBuff, INPUT_TEXT_BUFF_SIZE);

	ImGui::Separator();
	ImGui::Button("Create Account");
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		displayMode = DM_LOGIN;

	ShowRequestsStatus();
	ImGui::End();
}

void ValkyrieLoader::DisplayUserPanel()
{
	if (performUpdate) {
		std::shared_ptr<AsyncRequest> request(new AsyncRequest());
		request->response = api.GetCheatS3Object("valkyrie-releases-eu-north-1", "latest.zip");
		request->onSuccess = [this](std::shared_ptr<BaseAPIResponse>& response) {
			auto resp = (GetS3ObjectResponse*)response.get();
			std::thread downloadThread([this, &resp]() {
				UpdateValkyrie(resp);
			});

			downloadThread.detach();
		};

		asyncRequests["checking for updates"] = request;
		performUpdate = false;
	}

	/// Greeting
	ImGui::Begin("Valkyrie", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Welcome %s !", loggedUser.name.c_str());

	float days = (loggedUser.expiry - duration_cast<seconds>(system_clock::now().time_since_epoch()).count()) / ONE_DAY_SECS;
	float hours = (days - int(days)) * 24.f;

	ImGui::TextColored((days < 5.f ? COLOR_YELLOW : COLOR_GREEN), "Your subscription will expire in %d days %d hours", int(days), int(hours));

	/// Change log
	if (changeLog.size() > 0) {
		ImGui::Separator();
		ImGui::TextColored(COLOR_PURPLE, "** Change Log **");
		ImGui::BeginChildFrame(10000, ImVec2(400.f, 300.f));
		ImGui::Text(changeLog.c_str());
		ImGui::EndChildFrame();
	}

	/// Injection related
	ImGui::Separator();
	ImGui::Text("No league process active.");
	ImGui::Button("Inject Valkyrie");

	ShowRequestsStatus();
	ShowUpdateStatus();

	ImGui::End();
}

void ValkyrieLoader::DisplayAdminPanel()
{
	ImGui::ShowDemoWindow();
	if (retrieveUsers) {
		std::shared_ptr<AsyncRequest> request(new AsyncRequest());
		request->response = api.GetUsers(IdentityInfo(nameBuff, passBuff, hardwareInfo));
		request->onSuccess = [this](std::shared_ptr<BaseAPIResponse>& response) {
			auto resp = (GetUserListInfoResponse*)response.get();
			allUsers = resp->users;
			selectedUser = 0;
		};

		asyncRequests["getting users"] = request;
		retrieveUsers = false;
	}

	ImGui::Begin("Admin Panel");
	ImGui::PushItemWidth(100.f);

	/// Show user manager
	ImGui::Separator();
	ImGui::TextColored(COLOR_PURPLE, "All users");
	ImGui::BeginTable("Users tbl", 9, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);

	ImGui::TableSetupColumn("Name");
	ImGui::TableSetupColumn("Discord");
	ImGui::TableSetupColumn("Status");
	ImGui::TableSetupColumn("Privilege");
	ImGui::TableSetupColumn("Subscription");

	ImGui::TableSetupColumn("CPU");
	ImGui::TableSetupColumn("GPU");
	ImGui::TableSetupColumn("RAM");
	ImGui::TableSetupColumn("SYSTEM");

	ImGui::TableHeadersRow();

	float timestampNow = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < allUsers.size(); ++i) {
		auto& user = allUsers[i];

		ImGui::TableNextRow();
		ImGui::PushID(100 + i);

		/// Name
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable("", selectedUser == i, ImGuiSelectableFlags_SpanAllColumns))
			selectedUser = i;

		ImGui::SameLine();
		ImGui::Text(user.name.c_str());

		/// Discord
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(user.discord.c_str());

		/// Status
		ImGui::TableSetColumnIndex(2);
		if (user.locked)
			ImGui::TextColored(COLOR_RED, "Banned");
		else
			ImGui::TextColored(COLOR_GREEN, "Not banned");

		/// Privilege
		ImGui::TableSetColumnIndex(3);
		if (user.level == 0)
			ImGui::Text("User");
		else
			ImGui::TextColored(COLOR_PURPLE, "Super User");

		/// Expiry
		ImGui::TableSetColumnIndex(4);
		float days = (user.expiry - timestampNow) / ONE_DAY_SECS;
		float hours = (days - int(days)) * 24.f;
		ImGui::TextColored((days < 0.f ? COLOR_RED : (days < 5.f ? COLOR_YELLOW : COLOR_GREEN)), "%d days %d hours", int(days), int(hours));

		///Hardware
		ImGui::TableSetColumnIndex(5);
		ImGui::Text(user.hardware.cpuInfo.c_str());

		ImGui::TableSetColumnIndex(6);
		ImGui::Text(user.hardware.gpuInfo.c_str());

		ImGui::TableSetColumnIndex(7);
		ImGui::Text(user.hardware.ramInfo.c_str());

		ImGui::TableSetColumnIndex(8);
		ImGui::Text(user.hardware.systemName.c_str());

		ImGui::PopID();
	}
	ImGui::EndTable();

	ImGui::TextColored(COLOR_PURPLE, "User actions");
	ImGui::DragFloat("Subscription days to add", &deltaDays);
	ImGui::Button("Add days to selected");
	ImGui::SameLine();
	ImGui::Button("Add days to all");
	ImGui::SameLine();
	ImGui::Button("Ban/Unban Selected");
	ImGui::SameLine();
	ImGui::Button("HWID Reset Selected");

	/// Show invite code generator
	ImGui::Separator();
	ImGui::TextColored(COLOR_PURPLE, "Invite code generator");
	ImGui::DragFloat("Subscription Days", &inviteSubscriptionDays);
	ImGui::InputText("Generated Code", generatedInviteCodeBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::Button("Generate code")) {
		/// bla bla bla
		strcpy_s(generatedInviteCodeBuff, "generated bleahhh");
	}

	ImGui::PopItemWidth();
	ImGui::End();
}
