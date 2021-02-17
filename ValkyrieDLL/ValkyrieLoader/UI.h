#pragma once
#include "ValkyrieAPI.h"

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_PANEL
};

class UI {

public:

	void ImGuiShow();

private:

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayPanel();

	ValkyrieAPI      api;
	DisplayMode      displayMode = DM_LOGIN;

	/// Login stuff
	static const int INPUT_TEXT_BUFF_SIZE            = 256;
	char             nameBuff[INPUT_TEXT_BUFF_SIZE]  = "your_username";
	char             passBuff[INPUT_TEXT_BUFF_SIZE]  = "<pass>";
	AuthResponse     authResponse;

	/// Create account stuff
	char             passConfirmBuff[INPUT_TEXT_BUFF_SIZE] = "<pass>";
	char             discordBuff[INPUT_TEXT_BUFF_SIZE]     = "your_discord#0000";
	char             inviteCodeBuff[INPUT_TEXT_BUFF_SIZE]  = "your_invite_code";

};