#pragma once

enum DisplayMode {
	DM_LOGIN,
	DM_CREATE_ACCOUNT,
	DM_LOGGED_IN
};

class UI {

public:

	void ImGuiShow();

private:

	void DisplayLogin();
	void DisplayCreateAccount();
	void DisplayLoggedIn();

	DisplayMode      displayMode = DM_LOGIN;

	/// Login stuff
	static const int INPUT_TEXT_BUFF_SIZE            = 256;
	char             nameBuff[INPUT_TEXT_BUFF_SIZE]  = "your_username";
	char             passBuff[INPUT_TEXT_BUFF_SIZE]  = "<pass>";

	/// Create account stuff
	char             passConfirmBuff[INPUT_TEXT_BUFF_SIZE] = "<pass>";
	char             discordBuff[INPUT_TEXT_BUFF_SIZE]     = "your_discord#0000";
	char             inviteCodeBuff[INPUT_TEXT_BUFF_SIZE]  = "your_invite_code";

};