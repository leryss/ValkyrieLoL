#include "UI.h"
#include "imgui/imgui.h"

void UI::ImGuiShow()
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

void UI::DisplayLogin()
{
	ImGui::Begin("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Name", nameBuff, INPUT_TEXT_BUFF_SIZE);
	ImGui::InputText("Password", passBuff, INPUT_TEXT_BUFF_SIZE, ImGuiInputTextFlags_Password);

	ImGui::Separator();
	if (ImGui::Button("Login")) {
		authResponse = api.Authorize(nameBuff, passBuff, 60.f*60.f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Use invite code"))
		displayMode = DM_CREATE_ACCOUNT;

	if (authResponse != nullptr) {
		if (authResponse->status == RS_FAILURE) {
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Failed to login:");
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), authResponse->error.c_str());
		}

		if (authResponse->status == RS_EXECUTING) {
			ImGui::Text("Logging in");
		}

		if (authResponse->status == RS_SUCCESS) {
			displayMode = DM_PANEL;
		}
	}

	ImGui::End();
}

void UI::DisplayCreateAccount()
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

	ImGui::End();
}

void UI::DisplayPanel()
{
	ImGui::Begin("Valkyrie", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Welcome <user> !");
	ImGui::Text("Your subscription will expire on <date>");

	ImGui::Separator();
	ImGui::Text("No league process active.");
	ImGui::Button("Inject Valkyrie");

	ImGui::End();
}
