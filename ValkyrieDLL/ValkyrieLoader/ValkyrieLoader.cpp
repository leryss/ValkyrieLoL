#include "ValkyrieLoader.h"

#include "imgui/imgui.h"
#include "miniz/miniz.h"

#include <fstream>
#include <windows.h>
#include <shlobj_core.h>

static ImVec4 COLOR_RED = ImVec4(1.f, 0.f, 0.f, 1.f);
static ImVec4 COLOR_PURPLE = ImVec4(0.5f, 0.3f, 0.8f, 1.f);
static ImVec4 COLOR_GREEN = ImVec4(0.f, 1.f, 0.f, 1.f);
static const int MB_IN_BYTES = 1000000;

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
		std::ifstream versionFile("version");
		std::getline(versionFile, versionHash);
	}
}

void ValkyrieLoader::ImGuiShow()
{
	auto& io = ImGui::GetIO();

	switch (displayMode) {
	case DM_LOGIN:
		DisplayLogin();
		break;
	case DM_PANEL:
		DisplayPanel();
		break;
	case DM_CREATE_ACCOUNT:
		DisplayCreateAccount();
		break;
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
			ImGui::TextColored(COLOR_RED, "Failed to %s: %s", operationName, req->response->error.c_str());
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

void ValkyrieLoader::UpdateValkyrie(Aws::IOStream & updateFileStream)
{
	updater = std::shared_ptr<UpdaterProgress>(new UpdaterProgress(updateFileStream));
	
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
		updater->error  = "Failed to open update archive";
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
		request->response      = api.Authorize(nameBuff, passBuff, 60.f*60.f);
		request->onSuccess     = [this](std::shared_ptr<BaseAPIResponse>& response) {
			apiToken    = ((AuthResponse*)response.get())->token;
			displayMode = DM_PANEL;
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

void ValkyrieLoader::DisplayPanel()
{
	if (performUpdate) {
		std::shared_ptr<AsyncRequest> request(new AsyncRequest());
		request->response = api.GetCheatS3Object("valkyrie-releases-eu-north-1", "latest.zip");
		request->onSuccess = [this](std::shared_ptr<BaseAPIResponse>& response) {
			auto resp = (GetS3ObjectResponse*)response.get();

			std::thread downloadThread([this, &resp]() {
				UpdateValkyrie(resp->result.GetBody());
			});

			downloadThread.detach();
		};

		asyncRequests["checking for updates"] = request;
		performUpdate = false;
	}

	ImGui::Begin("Valkyrie", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Welcome <user> !");
	ImGui::Text("Your subscription will expire on <date>");

	ImGui::Separator();
	ImGui::Text("No league process active.");
	ImGui::Button("Inject Valkyrie");

	ShowRequestsStatus();
	ShowUpdateStatus();

	ImGui::End();
}
