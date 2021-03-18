#include "LoginPanel.h"
#include "ValkyrieShared.h"
#include "ValkyrieLoader.h"

void LoginPanel::Draw(ValkyrieLoader & loader)
{
	if (ImGui::Begin("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::InputText("Name", nameBuff, Constants::INPUT_TEXT_SIZE);
		ImGui::InputText("Password", passBuff, Constants::INPUT_TEXT_SIZE, ImGuiInputTextFlags_Password);

		ImGui::Separator();
		if (loadCredentials) {
			ValkyrieShared::LoadCredentials(nameBuff, passBuff, Constants::INPUT_TEXT_SIZE);
			if(autoLogin) {
				Login(loader);
			}
			loadCredentials = false;
		}

		if ((ImGui::Button("Login")) && !taskPool->IsExecuting(trackIdLogin)) {
			Login(loader);
		}
		ImGui::SameLine();
		if (ImGui::Button("Register"))
			loader.ChangePanel(&loader.createAccPanel);
		ImGui::SameLine();
		if (ImGui::Button("Extend Subscription"))
			loader.ChangePanel(&loader.extendSubPanel);

		ImGui::Checkbox("Remember credentials", &autoSaveCredentials);
		ImGui::Checkbox("Auto login", &autoLogin);
		ImGui::End();
	}
}

void LoginPanel::Login(ValkyrieLoader& loader)
{
	loader.identity = IdentityInfo(nameBuff, passBuff, loader.hardwareInfo);
	taskPool->DispatchTask(
		trackIdLogin,
		api->GetUser(loader.identity, nameBuff),
		[&loader, this](std::shared_ptr<AsyncTask> response) {
			ValkyrieShared::SaveCredentials(nameBuff, passBuff);
			loader.loggedUser = ((UserResultAsync*)response.get())->user;
			loader.ChangePanel(&loader.userPanel);
		}
	);
}