#include "CreateAccountPanel.h"
#include "ValkyrieLoader.h"
#include "ValkyrieShared.h"

void CreateAccountPanel::Draw(ValkyrieLoader & loader)
{
	if (ImGui::Begin("Create account", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::InputText("Invite Code",      inviteCodeBuff,             Constants::INPUT_TEXT_SIZE);
		ImGui::InputText("Name",             loader.loginPanel.nameBuff, Constants::INPUT_TEXT_SIZE);
		ImGui::InputText("Password",         loader.loginPanel.passBuff, Constants::INPUT_TEXT_SIZE, ImGuiInputTextFlags_Password);
		ImGui::InputText("Confirm Password", passConfirmBuff,            Constants::INPUT_TEXT_SIZE, ImGuiInputTextFlags_Password);
		ImGui::InputText("Discord",          discordBuff,                Constants::INPUT_TEXT_SIZE);

		ImGui::Separator();
		if (ImGui::Button("Create Account") && !taskPool->IsExecuting(trackIdCreateAccount)) {
			taskPool->DispatchTask(
				trackIdCreateAccount,
				api->CreateAccount(loader.loginPanel.nameBuff, loader.loginPanel.passBuff, discordBuff, loader.hardwareInfo, inviteCodeBuff),
				[&loader, this](std::shared_ptr<AsyncTask> response) {
					ValkyrieShared::SaveCredentials(loader.loginPanel.nameBuff, loader.loginPanel.passBuff);
					loader.loggedUser   = ((UserResultAsync*)response.get())->user;
					loader.identity     = IdentityInfo(loader.loginPanel.nameBuff, loader.loginPanel.passBuff, loader.hardwareInfo);
					loader.currentPanel = &loader.userPanel;
			}
			);
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			loader.currentPanel = &loader.loginPanel;

		ImGui::End();
	}
}
