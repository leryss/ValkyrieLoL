#include "AdminPanel.h"
#include "ValkyrieLoader.h"

enum UserColumnId {
	UserColumnName,
	UserColumnDiscord,
	UserColumnStatus,
	UserColumnPrivilege,
	UserColumnSubscription,
	UserColumnOther
};

static const ImGuiTableSortSpecs* CurrentSortSpecs;
static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
{
	const UserInfo* a = (const UserInfo*)lhs;
	const UserInfo* b = (const UserInfo*)rhs;
	for (int n = 0; n < CurrentSortSpecs->SpecsCount; n++)
	{
		// Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
		// We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
		const ImGuiTableColumnSortSpecs* sort_spec = &CurrentSortSpecs->Specs[n];
		int delta = 0;
		switch (sort_spec->ColumnUserID)
		{
		case UserColumnName:            delta = (strcmp(a->name.c_str(), b->name.c_str()));       break;
		case UserColumnDiscord:         delta = (strcmp(a->discord.c_str(), b->discord.c_str())); break;
		case UserColumnStatus:          delta = a->locked || b->locked;                           break;
		case UserColumnPrivilege:       delta = a->level - b->level;                              break;
		case UserColumnSubscription:    delta = a->expiry - b->expiry;                            break;
		default: IM_ASSERT(0); break;
		}
		if (delta > 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
		if (delta < 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
	}

	// qsort() is instable so always return a way to differenciate items.
	// Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
	return (strcmp(a->name.c_str(), b->name.c_str()));
}

void AdminPanel::Draw(ValkyrieLoader & loader)
{
	if (ImGui::Begin("Admin Panel")) {
		this->loader = &loader;
		ImGui::PushItemWidth(140.f);

		if (ImGui::BeginTabBar("AdminTabBar")) {
			if (ImGui::BeginTabItem("Users")) {
				DrawUserManager();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Invite Generator")) {
				DrawInviteGenerator();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Submission Approver")) {
				DrawSubmissionManager();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void AdminPanel::DrawSubmissionManager()
{
	if (RetrieveSubmissionsIfNecessary())
		return;

	if (ImGui::Button("Refresh"))
		retrieveSubmissions = true;

	mtxSubmissions.lock();

	ImGui::BeginTable("Users tbl", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
	ImGui::TableSetupColumn("Id");
	ImGui::TableSetupColumn("Name");
	ImGui::TableSetupColumn("Author");
	ImGui::TableSetupColumn("Champion");
	ImGui::TableSetupColumn("Description");
	ImGui::TableHeadersRow();

	for (size_t i = 0; i < submissions.size(); ++i) {
		auto& submission = submissions[i];
		auto& script = submission->script;
		if (submission->status != SUBMISSION_PENDING)
			continue;

		ImGui::TableNextRow();
		ImGui::PushID(i);

		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable("", selectedSubmission == i, ImGuiSelectableFlags_SpanAllColumns)) {
			retrieveCode = true;
			selectedSubmission = i;
		}

		ImGui::SameLine();
		ImGui::Text(script->id.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::Text(script->name.c_str());

		ImGui::TableSetColumnIndex(2);
		ImGui::Text(script->author.c_str());

		ImGui::TableSetColumnIndex(3);
		ImGui::Text(script->champion.c_str());

		ImGui::TableSetColumnIndex(4);
		ImGui::Text(script->description.c_str());

		ImGui::PopID();
	}

	ImGui::EndTable();

	DrawSubmissionManagerActions();

	mtxSubmissions.unlock();
}

void AdminPanel::DrawSubmissionManagerActions()
{
	if (selectedSubmission != -1 && !RetrieveScriptCodeIfNecessary()) {
		auto submission = submissions[selectedSubmission];
		bool update = false;

		if (ImGui::Button("Approve")) {
			submission->status = SUBMISSION_APPROVED;
			UpdateSubmission(submission);
		}
		ImGui::SameLine();
		if (ImGui::Button("Deny")) {
			ImGui::OpenPopup("DenyReason");
		}

		if (ImGui::BeginPopupModal("DenyReason")) {
			ImGui::PushItemWidth(500);
			ImGui::InputText("Deny reason", submissionDenyReason, Constants::INPUT_TEXT_SIZE);
			ImGui::PopItemWidth();

			if (ImGui::Button("Ok")) {
				submission->denyReason = std::string(submissionDenyReason);
				submission->status = SUBMISSION_DENIED;
				UpdateSubmission(submission);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		ImGui::TextColored(Color::PURPLE, "Script Code");
		ImGui::TextUnformatted(submissionCode.c_str());
	}
}

void AdminPanel::DrawUserManager()
{
	//ImGui::ShowStyleEditor();
	RetrieveUsersIfNecessarry();

	ImGui::Separator();
	ImGui::TextColored(Color::PURPLE, "All users");

	DrawUserManagerFilter();
	ImGui::BeginTable("UsersTable", 9, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable , ImVec2(0.f, 250.f));
	ImGui::TableSetupColumn("Name",         ImGuiTableColumnFlags_None,                                         0, UserColumnName);
	ImGui::TableSetupColumn("Discord",      ImGuiTableColumnFlags_None,                                         0, UserColumnDiscord);
	ImGui::TableSetupColumn("Status",       ImGuiTableColumnFlags_None,                                         0, UserColumnStatus);
	ImGui::TableSetupColumn("Privilege",    ImGuiTableColumnFlags_None,                                         0, UserColumnPrivilege);
	ImGui::TableSetupColumn("Subscription", ImGuiTableColumnFlags_None,                                         0, UserColumnSubscription);
	ImGui::TableSetupColumn("CPU",          ImGuiTableColumnFlags_NoSort,                                       0, UserColumnOther);
	ImGui::TableSetupColumn("GPU",          ImGuiTableColumnFlags_NoSort,                                       0, UserColumnOther);
	ImGui::TableSetupColumn("RAM",          ImGuiTableColumnFlags_NoSort,                                       0, UserColumnOther);
	ImGui::TableSetupColumn("SYSTEM",       ImGuiTableColumnFlags_NoSort,                                       0, UserColumnOther);
	ImGui::TableHeadersRow();

	SortUsersIfNecessarry();

	float timestampNow = (float)duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	for (size_t i = 0; i < allUsers.size(); ++i) {
		if (!filterMask[i])
			continue;

		auto& user = allUsers[i];

		ImGui::TableNextRow();
		ImGui::PushID(100 + i);

		/// Name
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable("", selectedUser == i, ImGuiSelectableFlags_SpanAllColumns)) {
			selectedUser = i;
			selectedRole = allUsers[selectedUser].level;
		}

		ImGui::SameLine();
		ImGui::Text(user.name.c_str());

		/// Discord
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(user.discord.c_str());

		/// Status
		ImGui::TableSetColumnIndex(2);
		if (user.locked)
			ImGui::TextColored(Color::RED, "Banned");
		else
			ImGui::TextColored(Color::GREEN, "Not banned");

		/// Privilege
		ImGui::TableSetColumnIndex(3);
		switch ((int)user.level) {
		case USER_LEVEL_NORMAL:
			ImGui::Text("User");
			break;
		case USER_LEVEL_TESTER:
			ImGui::TextColored(Color::YELLOW, "Tester");
			break;
		case USER_LEVEL_ADMIN:
			ImGui::TextColored(Color::BLUE, "Admin");
			break;
		case USER_LEVEL_SUPER_ADMIN:
			ImGui::TextColored(Color::PURPLE, "Super Admin");
			break;
		}

		/// Expiry
		ImGui::TableSetColumnIndex(4);
		float days = (user.expiry - timestampNow) / Constants::ONE_DAY_SECS;
		float hours = (days - int(days)) * 24.f;
		ImGui::TextColored((days < 0.f ? Color::RED : (days < 5.f ? Color::YELLOW : Color::GREEN)), "%d days %d hours", int(days), int(hours));

		///Hardware
		ImGui::TableSetColumnIndex(5);
		ImGui::Text(user.hardware.cpuInfo.c_str());

		ImGui::TableSetColumnIndex(6);
		ImGui::Text(user.hardware.gpuInfo.c_str());

		ImGui::TableSetColumnIndex(7);
		ImGui::Text(user.hardware.ramInfo.c_str());

		ImGui::TableSetColumnIndex(8);
		ImGui::Text(user.hardware.systemName.c_str());

		ImGui::PopID();
	}
	ImGui::EndTable();

	DrawUserManagerActions();
}

void AdminPanel::DrawUserManagerFilter()
{
	if (ImGui::InputText("Filter", userFilter, Constants::INPUT_TEXT_SIZE)) {
		std::string filter(userFilter);
		std::string filterLower = Strings::ToLower(filter);

		for (size_t i = 0; i < allUsers.size(); ++i) {
			auto& user = allUsers[i];
			std::string nameLower = Strings::ToLower(user.name);
			std::string discordLower = Strings::ToLower(user.discord);

			filterMask[i] = (nameLower.find(filterLower) != nameLower.npos || discordLower.find(filterLower) != discordLower.npos);
		}
	}
}

void AdminPanel::DrawUserManagerActions()
{
	if (RetrieveUsersIfNecessarry())
		return;

	mtxUsers.lock();

	if (ImGui::Button("Refresh")) {
		selectedUser = 0;
		retrieveUsers = true;
	}

	if (taskPool->IsExecuting(trackIdUpdateUser)) {
		ImGui::TextColored(Color::YELLOW, "Performing action...");
		mtxUsers.unlock();
		return;
	}

	bool  doUpdate = false;
	auto& selected = allUsers[selectedUser];

	ImGui::TextColored(Color::PURPLE, "User actions");
	ImGui::DragFloat("Subscription days to add", &deltaDays);
	if (ImGui::Combo("Account role", &selectedRole, "User\0Tester\0Admin\0Super Admin")) {
		doUpdate = true;
		selected.level = (float)selectedRole;
	}

	if (ImGui::Button("Add days to selected")) {
		doUpdate = true;
		selected.expiry += Constants::ONE_DAY_SECS * deltaDays;
	}

	ImGui::SameLine();
	if (ImGui::Button("Ban/Unban Selected")) {
		doUpdate = true;
		selected.locked = !selected.locked;
	}

	ImGui::SameLine();
	if (ImGui::Button("HWID Reset Selected")) {
		doUpdate = true;
		selected.resetHardware = true;
	}

	if (doUpdate) {
		int toReplace = selectedUser;
		taskPool->DispatchTask(
			trackIdUpdateUser,
			api->UpdateUser(loader->identity, selected.name.c_str(), selected),
			[this, toReplace](std::shared_ptr<AsyncTask> response) {
				mtxUsers.lock();
				allUsers[toReplace] = ((UserResultAsync*)response.get())->user;
				mtxUsers.unlock();
			}
		);
	}

	mtxUsers.unlock();
}

void AdminPanel::DrawInviteGenerator()
{
	ImGui::Separator();
	ImGui::TextColored(Color::PURPLE, "Invite code generator");

	ImGui::Combo("Role", &inviteRole, "User\0Tester\0Admin\0Super Admin");
	ImGui::DragFloat("Subscription Days", &inviteSubscriptionDays);
	ImGui::InputText("Generated Code", generatedInviteCodeBuff, Constants::INPUT_TEXT_SIZE, ImGuiInputTextFlags_ReadOnly);

	if (ImGui::Button("Generate code") && !taskPool->IsExecuting(trackIdGenerateInvite)) {
		taskPool->DispatchTask(
			trackIdGenerateInvite,
			api->GenerateInviteCode(loader->identity, inviteSubscriptionDays, (UserLevel)inviteRole),
			[this](std::shared_ptr<AsyncTask> response) {
				auto resp = (StringResultAsync*)response.get();
				strcpy_s(generatedInviteCodeBuff, resp->result.c_str());
			}
		);
	}
}

void AdminPanel::UpdateSubmission(std::shared_ptr<ScriptSubmission> submission)
{
	taskPool->DispatchTask(
		trackIdUpdateSubmission,
		api->UpdateSubmission(loader->identity, *submission),
		[this, submission](std::shared_ptr<AsyncTask> response) {
			mtxSubmissions.lock();
			selectedSubmission = -1;
			mtxSubmissions.unlock();
		}
	);
}

bool AdminPanel::RetrieveSubmissionsIfNecessary()
{
	if (taskPool->IsExecuting(trackIdGetSubmissions)) {
		ImGui::TextColored(Color::YELLOW, "Refreshing...");
		return true;
	}

	if (retrieveSubmissions) {

		taskPool->DispatchTask(
			trackIdGetSubmissions,
			api->GetAllSubmissions(loader->identity),
			[this](std::shared_ptr<AsyncTask> response) {
				mtxSubmissions.lock();
				submissions = ((ScriptSubmissionsResultAsync*)response.get())->submissions;
				mtxSubmissions.unlock();
			}
		);
		retrieveSubmissions = false;
	}
}

bool AdminPanel::RetrieveScriptCodeIfNecessary()
{
	if (taskPool->IsExecuting(trackIdGetCode)) {
		ImGui::TextColored(Color::YELLOW, "Getting code...");
		return true;
	}

	if (retrieveCode) {

		std::string codeId = submissions[selectedSubmission]->script->id;
		codeId.append("_submission");

		taskPool->DispatchTask(
			trackIdGetCode,
			api->GetScriptCode(loader->identity, codeId),
			[this](std::shared_ptr<AsyncTask> response) {
				mtxSubmissions.lock();
				submissionCode = ((StringResultAsync*)response.get())->result.c_str();
				mtxSubmissions.unlock();
			}
		);
		retrieveCode = false;
	}
}

bool AdminPanel::RetrieveUsersIfNecessarry()
{
	if (taskPool->IsExecuting(trackIdGetUsers)) {
		ImGui::TextColored(Color::YELLOW, "Refreshing...");
		return true;
	}

	if (retrieveUsers) {
		taskPool->DispatchTask(
			trackIdGetUsers,
			api->GetUsers(loader->identity),

			[this](std::shared_ptr<AsyncTask> response) {
				mtxUsers.lock();
				auto resp = (GetUserListAsync*)response.get();
				allUsers = resp->users;
				filterMask.clear();
				for (size_t i = 0; i < allUsers.size(); ++i)
					filterMask.push_back(true);

				selectedUser = 0;
				mtxUsers.unlock();
			}
		);
		retrieveUsers = false;
	}

	return false;
}

void AdminPanel::SortUsersIfNecessarry()
{
	if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
		if (sortSpecs->SpecsDirty)
		{
			CurrentSortSpecs = sortSpecs; // Store in variable accessible by the sort function.
			if (allUsers.size() > 1)
				qsort(&allUsers[0], allUsers.size(), sizeof(UserInfo), CompareWithSortSpecs);
			CurrentSortSpecs = NULL;
			sortSpecs->SpecsDirty = false;
		}
}
