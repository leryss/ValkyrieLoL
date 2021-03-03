#pragma once
#include <chrono>
#include <cstdlib>
#include <mutex>
#include "ScriptSubmission.h"
#include "LoaderPanel.h"

using namespace std::chrono;

class AdminPanel : LoaderPanel {

public:
	virtual void Draw(ValkyrieLoader& loader);

	void         DrawSubmissionManager();
	void         DrawSubmissionManagerActions();
	void         DrawUserManager();
	void         DrawUserManagerFilter();
	void         DrawUserManagerActions();
	void         DrawInviteGenerator();

	void         UpdateSubmission(std::shared_ptr<ScriptSubmission> submission);
	bool         RetrieveSubmissionsIfNecessary();
	bool         RetrieveScriptCodeIfNecessary();
	bool         RetrieveUsersIfNecessarry();
	void         SortUsersIfNecessarry();

public:
	std::vector<UserInfo> allUsers;

private:

	std::string trackIdGetCode          = "GetCode";
	std::string trackIdGetSubmissions   = "GetSubmissions";
	std::string trackIdUpdateSubmission = "UpdateSubmission";
	std::string trackIdUpdateUser       = "UpdateUser";
	std::string trackIdGetUsers         = "GetUsers";
	std::string trackIdGenerateInvite   = "GenerateInvite";

	/// Submission manager stuff
	std::vector<std::shared_ptr<ScriptSubmission>> submissions;
	std::string                                    submissionCode;
	int                                            selectedSubmission = -1;
	bool                                           retrieveSubmissions = true;
	bool                                           retrieveCode = false;
	std::mutex                                     mtxSubmissions;
	char                                           submissionDenyReason[Constants::INPUT_TEXT_SIZE];

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
	std::mutex        mtxUsers;

	ValkyrieLoader*   loader;
};