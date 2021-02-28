#pragma once
#include "LoaderPanel.h"
#include "UserInfo.h"

class LoginPanel : public LoaderPanel {

public:
	virtual void     Draw(ValkyrieLoader& loader);

public:
	bool             autoSaveCredentials;
	bool             loadCredentials;

	char             nameBuff[Constants::INPUT_TEXT_SIZE] = "your_username";
	char             passBuff[Constants::INPUT_TEXT_SIZE] = "<pass>";

	std::string      trackIdLogin = "LogIn";
};