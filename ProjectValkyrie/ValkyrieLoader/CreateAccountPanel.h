#pragma once
#include "LoaderPanel.h"

class CreateAccountPanel : public LoaderPanel {

public:
	virtual void Draw(ValkyrieLoader& loader);

public:
	char passConfirmBuff[Constants::INPUT_TEXT_SIZE] = "<pass>";
	char discordBuff[Constants::INPUT_TEXT_SIZE] = "your_discord#0000";
	char inviteCodeBuff[Constants::INPUT_TEXT_SIZE] = "your_invite_code";

	std::string trackIdCreateAccount = "CreateAccount";
};