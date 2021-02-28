#pragma once
#include <chrono>
#include <cstdlib>
#include "LoaderPanel.h"

using namespace std::chrono;

class AdminPanel : LoaderPanel {

public:
	virtual void Draw(ValkyrieLoader& loader);

	void         DrawUserManager();
	void         DrawUserManagerFilter();
	void         DrawUserManagerActions();
	void         DrawInviteGenerator();

	void         RetrieveUsersIfNecessarry();
	void         SortUsersIfNecessarry();

public:
	std::vector<UserInfo> allUsers;

private:

	std::string trackIdUpdateUser     = "UpdateUser";
	std::string trackIdGetUsers       = "GetUsers";
	std::string trackIdGenerateInvite = "GenerateInvite";

	/// Invite code generator stuff
	int              inviteRole = 0;
	float            inviteSubscriptionDays = 30.f;
	char             generatedInviteCodeBuff[Constants::INPUT_TEXT_SIZE] = "";

	/// User Manager stuff
	bool              retrieveUsers = true;
	int               selectedRole = 0;
	int               selectedUser = 0;
	float             deltaDays = 2.f;
	char              userFilter[Constants::INPUT_TEXT_SIZE];
	std::vector<bool> filterMask;

	ValkyrieLoader*   loader;
};