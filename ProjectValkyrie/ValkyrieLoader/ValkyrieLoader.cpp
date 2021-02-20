#include "ValkyrieLoader.h"
#include "AsyncInjector.h"

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

static const float ONE_DAY_SECS = 60.f * 60.f * 24.f;

ValkyrieLoader::ValkyrieLoader()
	:taskPool(4)
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

	if (!SetEnvironmentVariable("VPath", valkyrieFolder.c_str()))
		throw std::exception("Fatal error. Couldn't set VPath");
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

	ShowRequestsStatus();
}

void ValkyrieLoader::ShowRequestsStatus()
{
	DWORD tickCount = GetTickCount() % 600;

	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::Begin("Tasks", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	ImGui::TextColored(COLOR_YELLOW, "Background tasks");

	taskPool.VisitTasks([this, &tickCount](std::string taskId, std::shared_ptr<AsyncTask>& task) {
		const char* operationName = taskId.c_str();

		switch (task->GetStatus()) {
		case ASYNC_RUNNING:
			ImGui::Separator();
			ImGui::TextColored(COLOR_PURPLE, operationName);
			ImGui::TextColored(COLOR_GREEN, "Executing step: %s%s", task->currentStep.c_str(), (tickCount < 200 ? "." : (tickCount < 400 ? ".." : "...")));
			break;
		case ASYNC_FAILED:
			ImGui::Separator();
			ImGui::TextColored(COLOR_PURPLE, operationName);
			ImGui::TextColored(COLOR_RED, "Step `%s` failed: %s", task->currentStep.c_str(), task->error.c_str());
			break;
		}
	});

	ImGui::End();
}

void ValkyrieLoader::DisplayLogin()
{
	ImGui::Begin("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Name", nameBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Password", passBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);

	ImGui::Separator();
	if (ImGui::Button("Login") && !taskPool.IsExecuting(trackIdLogin)) {
		taskPool.DispatchTask(
			trackIdLogin,
			api.GetUser(IdentityInfo(nameBuff, passBuff, hardwareInfo), nameBuff),
			[this](std::shared_ptr<AsyncTask> response) {
				loggedUser = ((GetUserAsync*)response.get())->user;
				displayMode = DM_USER_PANEL;
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Button("Use invite code"))
		displayMode = DM_CREATE_ACCOUNT;

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
	if (ImGui::Button("Create Account") && !taskPool.IsExecuting(trackIdCreateAccount)) {
		taskPool.DispatchTask(
			trackIdCreateAccount,
			api.CreateAccount(nameBuff, passBuff, discordBuff, hardwareInfo, inviteCodeBuff),
			[this](std::shared_ptr<AsyncTask> response) {
				loggedUser = ((CreateAccountAsync*)response.get())->user;
				displayMode = DM_USER_PANEL;
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		displayMode = DM_LOGIN;

	ImGui::End();
}

void ValkyrieLoader::DisplayUserPanel()
{
	if (performUpdate && !taskPool.IsExecuting(trackIdCheckVersion)) {
		taskPool.DispatchTask(
			trackIdCheckVersion,
			api.GetCheatS3Object("valkyrie-releases-eu-north-1", "latest.zip"),

			[this](std::shared_ptr<AsyncTask> response) {
				taskPool.DispatchTask(
					trackIdUpdate, 
					std::shared_ptr<AsyncTask>((AsyncTask*)new UpdaterAsync(*this, std::static_pointer_cast<GetS3ObjectAsync>(response))),
					[this](std::shared_ptr<AsyncTask> response) {
						
						taskPool.DispatchTask(
							trackIdInjector,
							std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncInjector({ valkyrieFolder + DLL_PATH_PYTHON, valkyrieFolder + DLL_PATH_VALKYRIE })),
							[this](std::shared_ptr<AsyncTask> response) {} /// This task is not supposed to finish
						);
					}
				);
			}
		);
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

	ImGui::End();
}

void ValkyrieLoader::DisplayAdminPanel()
{
	ImGui::ShowDemoWindow();
	if (retrieveUsers && !taskPool.IsExecuting(trackIdGetUsers)) {
		taskPool.DispatchTask(
			trackIdGetUsers,
			api.GetUsers(IdentityInfo(nameBuff, passBuff, hardwareInfo)),

			[this](std::shared_ptr<AsyncTask> response) {
				auto resp = (GetUserListAsync*)response.get();
				allUsers = resp->users;
				selectedUser = 0;
			}
		);
		retrieveUsers = false;
	}

	ImGui::Begin("Admin Panel");
	ImGui::PushItemWidth(140.f);

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

	if (ImGui::Button("Generate code") && !taskPool.IsExecuting(trackIdGenerateInvite)) {
		taskPool.DispatchTask(
			trackIdGenerateInvite,
			api.GenerateInviteCode(IdentityInfo(nameBuff, passBuff, hardwareInfo), inviteSubscriptionDays),
			[this](std::shared_ptr<AsyncTask> response) {
				auto resp = (GenerateInviteAsync*)response.get();
				strcpy_s(generatedInviteCodeBuff, resp->inviteCode.c_str());
			}
		);
		
	}

	ImGui::PopItemWidth();
	ImGui::End();
}

UpdaterAsync::UpdaterAsync(ValkyrieLoader & vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	:loader(vloader), updateFile(s3UpdateFile)
{}

void UpdaterAsync::Perform()
{
	auto& updateFileStream = updateFile->result.GetBody();

	/// Check if there is a new version and download the files
	auto& etag = updateFile->result.GetETag();
	if (etag.compare(loader.versionHash.c_str()) != 0) {

		/// Download from stream
		currentStep = "Downloading data";

		updateFileStream.seekg(0, updateFileStream.end);
		int sizeDownloadFull = updateFileStream.tellg();
		updateFileStream.seekg(0, updateFileStream.beg);

		int   sizeDownloaded = 0;
		char* downloadBuff = new char[sizeDownloadFull];
		while (true) {
			std::streamsize readBytes = updateFileStream.readsome(downloadBuff + sizeDownloaded, 10000);
			sizeDownloaded += readBytes;
			if (readBytes == 0)
				break;
		}

		/// Extract from downloaded archive
		currentStep = "Extracting data";

		mz_zip_archive archive;
		memset(&archive, 0, sizeof(archive));
		if (!mz_zip_reader_init_mem(&archive, downloadBuff, sizeDownloaded, 0)) {
			SetError("Failed to open update archive");
			delete[] downloadBuff;
			return;
		}

		int numFilesToExtract = mz_zip_reader_get_num_files(&archive);
		for (int i = 0; i < numFilesToExtract; ++i) {
			mz_zip_archive_file_stat fileStat;
			if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
				auto err = std::string("Failed to get archive file info for file num ") + std::to_string(i);
				SetError(err.c_str());
				delete[] downloadBuff;
				return;
			}

			std::string path = loader.valkyrieFolder + "\\" + fileStat.m_filename;
			if (fileStat.m_is_directory) {
				CreateDirectoryA(path.c_str(), NULL);
			}
			else if (!mz_zip_reader_extract_file_to_file(&archive, fileStat.m_filename, path.c_str(), 0)) {
				auto err = std::string("Failed to unarchive file ") + fileStat.m_filename;
				SetError(err.c_str());
				delete[] downloadBuff;
				return;
			}
		}

		mz_zip_reader_end(&archive);

		/// Update version hash
		loader.versionHash = etag.c_str();
		std::ofstream versionFile(loader.valkyrieFolder + "\\version");
		versionFile.write(loader.versionHash.c_str(), loader.versionHash.size());

		delete[] downloadBuff;
	}

	/// Read change logs
	std::ifstream changeLogFile(loader.valkyrieFolder + "\\changelog.txt");
	loader.changeLog = std::string((std::istreambuf_iterator<char>(changeLogFile)), std::istreambuf_iterator<char>());

	SetStatus(ASYNC_SUCCEEDED);
}
