#include "ValkyrieLoader.h"
#include "AsyncInjector.h"
#include "AsyncUpdater.h"
#include "ValkyrieShared.h"

#include "imgui/imgui.h"
#include "miniz/miniz.h"

#include <fstream>
#include <windows.h>
#include <chrono>
#include <cstdlib>

using namespace std::chrono;

static ImVec4      COLOR_RED    = ImVec4(1.f, 0.f, 0.f, 1.f);
static ImVec4      COLOR_PURPLE = ImVec4(0.5f, 0.3f, 0.8f, 1.f);
static ImVec4      COLOR_GREEN  = ImVec4(0.f, 1.f, 0.f, 1.f);
static ImVec4      COLOR_YELLOW = ImVec4(1.f, 1.f, 0.f, 1.f);

static const float ONE_DAY_SECS = 60.f * 60.f * 24.f;

ValkyrieLoader::ValkyrieLoader()
{
	taskPool.AddWorkers(4);

	SetupWorkingDir();
	ReadVersion();
	hardwareInfo = HardwareInfo::Calculate();
	LoadConfigs();
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

	taskPool.ImGuiDraw();
	if (configs.IsTimeToSave())
		SaveConfigs();
}

void ValkyrieLoader::DisplayLogin()
{
	ImGui::Begin("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Name", nameBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Password", passBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);

	ImGui::Separator();
	if (loadCredentials) {
		ValkyrieShared::LoadCredentials(nameBuff, passBuff, INPUT_TEXT_BUFF_SIZE);
		loadCredentials = false;
	}

	if ((ImGui::Button("Login")) && !taskPool.IsExecuting(trackIdLogin)) {

		taskPool.DispatchTask(
			trackIdLogin,
			api.GetUser(IdentityInfo(nameBuff, passBuff, hardwareInfo), nameBuff),
			[this](std::shared_ptr<AsyncTask> response) {
				ValkyrieShared::SaveCredentials(nameBuff, passBuff);
				loggedUser = ((UserOperationAsync*)response.get())->user;
				displayMode = DM_USER_PANEL;
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Button("Register"))
		displayMode = DM_CREATE_ACCOUNT;

	ImGui::Checkbox("Remember credentials", &autoSaveCredentials);

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
				loggedUser = ((UserOperationAsync*)response.get())->user;
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
					std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncUpdater(*this, std::static_pointer_cast<GetS3ObjectAsync>(response))),
					[this](std::shared_ptr<AsyncTask> response) {
						
						updateComplete = true;
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
		ImGui::BeginChildFrame(10000, ImVec2(400.f, 200.f));
		ImGui::Text(changeLog.c_str());
		ImGui::EndChildFrame();
	}

	/// Inject stuff
	ImGui::Separator();
	ImGui::Checkbox("Auto inject", &autoInject);
	if ((injectorTask == nullptr || injectorTask->GetStatus() != ASYNC_RUNNING) && updateComplete) {
		if (autoInject) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(valkyrieFolder + DLL_PATH_VALKYRIE, false));
			taskPool.DispatchTask(
				trackIdInjector,
				injectorTask,
				[this](std::shared_ptr<AsyncTask> response) {}
			);
		}
		else if (ImGui::Button("Inject")) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(valkyrieFolder + DLL_PATH_VALKYRIE, true));
			taskPool.DispatchTask(
				trackIdInjector,
				injectorTask,
				[this](std::shared_ptr<AsyncTask> response) {}
			);
		}
	}
	
	if (!autoInject && injectorTask != nullptr && injectorTask->GetStatus() == ASYNC_RUNNING)
		injectorTask->shouldStop = true;

	ImGui::End();
}

void ValkyrieLoader::DisplayAdminPanel()
{
	ImGui::Begin("Admin Panel");
	ImGui::PushItemWidth(140.f);

	DrawUserManager();
	DrawInviteGenerator();

	ImGui::PopItemWidth();
	ImGui::End();
}

void ValkyrieLoader::DrawUserManager()
{
	if (taskPool.IsExecuting(trackIdGetUsers)) {
		ImGui::TextColored(Color::YELLOW, "Refreshing...");
		return;
	}

	if (retrieveUsers) {
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

	float timestampNow = (float)duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	for (size_t i = 0; i < allUsers.size(); ++i) {
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

	if (ImGui::Button("Refresh")) {
		selectedUser = 0;
		retrieveUsers = true;
	}

	if (taskPool.IsExecuting(trackIdUpdateUser)) {
		ImGui::TextColored(Color::YELLOW, "Performing action...");
		return;
	}

	bool  doUpdate = false;
	auto& selected = allUsers[selectedUser];

	ImGui::TextColored(COLOR_PURPLE, "User actions");
	ImGui::DragFloat("Subscription days to add", &deltaDays);
	if (ImGui::Button("Add days to selected")) {
		doUpdate = true;
		selected.expiry += ONE_DAY_SECS * deltaDays;
	}

	ImGui::SameLine();
	if (ImGui::Button("Ban/Unban Selected")) {
		doUpdate = true;
		selected.locked = !selected.locked;
	}

	ImGui::SameLine();
	if (ImGui::Button("HWID Reset Selected")) {
		doUpdate = true;
		selected.resetHardware = true;
	}

	if (doUpdate) {
		int toReplace = selectedUser;
		taskPool.DispatchTask(
			trackIdUpdateUser,
			api.UpdateUser(IdentityInfo(nameBuff, passBuff, hardwareInfo), selected.name.c_str(), selected),
			[this, toReplace](std::shared_ptr<AsyncTask> response) {
				allUsers[toReplace] = ((UserOperationAsync*) response.get())->user;
			}
		);
	}
}

void ValkyrieLoader::DrawInviteGenerator()
{
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
}

void ValkyrieLoader::LoadConfigs()
{
	configs.SetSaveInterval(5000);
	configs.SetConfigFile("vload.cfg");
	configs.Load();

	autoSaveCredentials  = loadCredentials = configs.GetBool("save_credentials", false);
	autoInject = configs.GetBool("auto_inject", false);
}

void ValkyrieLoader::SaveConfigs()
{
	configs.SetBool("save_credentials", autoSaveCredentials);
	configs.SetBool("auto_inject", autoInject);

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
