#include "ScriptRepository.h"
#include "Strings.h"
#include "Paths.h"

#include <ctime>
#include <fstream>
#include <sstream>


std::string ScriptRepository::BaseCodeScript = ""
"from valkyrie import *\n"
"\n"
"def valkyrie_menu(ctx: Context):\n"
"	ui = ctx.ui\n"
"\n"
"def valkyrie_on_load(ctx: Context):\n"
"	cfg = ctx.cfg\n"
"	pass\n"
"\n"
"def valkyrie_on_save(ctx: Context):\n"
"	cfg = ctx.cfg\n"
"	pass\n"
"\n"
"def valkyrie_exec(ctx: Context):\n"
"	pass\n"
"\n"
"";

const char* ScriptRepository::Ratings[NumRatings] = {
	"Very Bad",
	"Bad",
	"Decent",
	"Good",
	"Very Good"
};

const ImVec4 ScriptRepository::ColorRatings[NumRatings] = {
	Color::RED,
	Color::ORANGE,
	Color::YELLOW,
	Color::GREEN,
	Color::GREEN
};

ScriptRepository::ScriptRepository()
	: comparator(entries)
{
}

void ScriptRepository::LoadLocalEntries(std::string path)
{
	mtxEntries.lock();

	std::ifstream stream;
	stream.open(path);

	if (!stream.is_open()) {
		mtxEntries.unlock();
		return;
	}

	JsonValue json(stream);

	auto jscripts = json.View().GetArray("installed");
	for (size_t i = 0; i < jscripts.GetLength(); ++i) {
		auto& jinfo = jscripts.GetItem(i);

		auto id = std::string(jinfo.GetString("id").c_str());
		auto script = ScriptInfo::FromJsonView(jinfo);
		AddEntry(script, true);
	}

	SortEntries();
	mtxEntries.unlock();
}

void ScriptRepository::SaveLocalEntries(std::string path)
{
	mtxEntries.lock();

	std::ofstream stream;
	stream.open(path, std::ofstream::out);

	if (!stream.is_open()) {
		throw std::exception(Strings::Format("Failed to open index file for reading: %s", path.c_str()).c_str());
	}

	std::vector<ScriptInfo*> locals;
	for (auto& pair : entries) {
		if (pair.second->local != nullptr) {
			locals.push_back(pair.second->local.get());
		}
	}

	JsonValue json;
	Aws::Utils::Array<JsonValue> jscripts(locals.size());
	for (size_t i = 0; i < locals.size(); ++i) {
		jscripts[i] = locals[i]->ToJsonValue();
	}

	json.WithArray("installed", jscripts);
	stream << json.View().WriteReadable();

	localsUnsaved = false;
	mtxEntries.unlock();
}

void ScriptRepository::LoadRemoteEntries(const IdentityInfo & identity)
{
	this->identity = identity;
	taskPool->DispatchTask(
		std::string("GetRemoteScripts"),
		api->GetScriptList(identity),
		[this](std::shared_ptr<AsyncTask> response) {

			/// Update repository with remote entries
			mtxEntries.lock();
			auto scripts = ((ScriptListAsync*)response.get())->scripts;
			for (auto& script : scripts)
				AddEntry(script, false);
			SortEntries();
			mtxEntries.unlock();

			/// Now get user submissions
			taskPool->DispatchTask(
				std::string("GetSubmissions"),
				api->GetSubmissions(this->identity, this->identity.name),
				[this](std::shared_ptr<AsyncTask> response) {
					mtxEntries.lock();
					UpdateSubmissions(((ScriptSubmissionsResultAsync*)response.get())->submissions);
					mtxEntries.unlock();
				}
			);
		}
	);
}

void ScriptRepository::Draw()
{
	mtxEntries.lock();

	if (ImGui::BeginTabBar("ScriptsTabBar")) {
		if (ImGui::BeginTabItem("My Scripts")) {

			if (ImGui::Button("Create New"))
				CreateLocalEntry();
			ImGui::SameLine();
			if (ImGui::Button("Open Scripts Folder"))
				ShellExecuteA(NULL, "open", Paths::Scripts.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			ImGui::SameLine();
			if (ImGui::Button("Update Installed"))
				UpdateInstalledScripts();
			DrawTable(true);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Online Scripts")) {
			DrawTable(false);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	mtxEntries.unlock();
}

std::shared_ptr<ScriptEntry> ScriptRepository::AddEntry(std::shared_ptr<ScriptInfo> & script, bool isLocal)
{
	auto find = entries.find(script->id);
	std::shared_ptr<ScriptEntry> entry;
	if (find != entries.end()) {
		entry = find->second;
	}
	else { 
		sorted.push_back(script->id);
		entry = std::shared_ptr<ScriptEntry>(new ScriptEntry);
		entries[script->id] = entry;
	}

	if (isLocal) {
		entry->local = script;
		localsUnsaved = true;
	}
	else {
		entry->remote = script;

		/// We update the local ratings to the remote ones
		if (entry->local != nullptr) {
			entry->local->averageRating = script->averageRating;
			entry->local->numRatings = script->numRatings;
		}
	}

	return entry;
}

void ScriptRepository::RemoveEntry(std::string id, bool isLocal, bool isBoth)
{
	auto find = entries.find(id);
	if (find == entries.end())
		return;

	if (isBoth || isLocal) {
		find->second->local = nullptr;
		localsUnsaved = true;
	}
	if (isBoth || !isLocal)
		find->second->remote = nullptr;

	if (find->second->local == nullptr && find->second->remote == nullptr)
		entries.erase(find);

	SortEntries();
}

void ScriptRepository::DrawTable(bool showLocal)
{
	static const float SECS_IN_DAY = 24.f * 60.f * 60.f;

	if (ImGui::InputText("Search", searchStr, 50)) {
		SortEntries();
	}

	ImGui::BeginTable("TableScripts", 8, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 400.f));
	ImGui::TableSetupColumn("Id",           ImGuiTableColumnFlags_None, 0, REPO_COLUMN_ID);
	ImGui::TableSetupColumn("Name",         ImGuiTableColumnFlags_None, 0, REPO_COLUMN_NAME);
	ImGui::TableSetupColumn("Author",       ImGuiTableColumnFlags_None, 0, REPO_COLUMN_AUTHOR);
	ImGui::TableSetupColumn("Champion",     ImGuiTableColumnFlags_DefaultSort, 0, REPO_COLUMN_CHAMP);
	ImGui::TableSetupColumn("Description",  ImGuiTableColumnFlags_None, 0);
	ImGui::TableSetupColumn("Last Update",  ImGuiTableColumnFlags_None, 0);
	ImGui::TableSetupColumn("Rating",       ImGuiTableColumnFlags_None, 0, REPO_COLUMN_RATING);
	ImGui::TableSetupColumn("Type",         ImGuiTableColumnFlags_None, 0, REPO_COLUMN_TYPE);
	
	if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs()) {
		comparator.sortSpecs = specs;
		if (comparator.sortSpecs->SpecsDirty)
		{
			SortEntries();
			comparator.sortSpecs->SpecsDirty = false;
		}
	}
	ImGui::TableHeadersRow();

	float nowTimestamp = (float)std::time(0);
	for (size_t i = 0; i < sorted.size(); ++i) {

		auto& id = sorted[i];
		auto& entry = entries[id];
		auto& script = (showLocal ? entry->local : entry->remote);
		if (script == nullptr)
			continue;

		if(GetTickCount() % 250 == 0)
			UpdateState(entry);

		ImGui::PushID(i);
		ImGui::TableNextRow();

		ImVec4 color = GetStateColor(entry->state);
		if (color.w > 0.f) {
			color.w = 0.35f;
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(color), i);
		}

		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable("", selectedScript == i, ImGuiSelectableFlags_SpanAllColumns))
			SelectEntry(i);
		ImGui::SameLine();
		ImGui::Text("%s.py", script->id.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::Text(script->name.c_str());

		ImGui::TableSetColumnIndex(2);
		ImGui::TextColored((script->author == "TeamValkyrie" ? Color::CYAN : Color::WHITE), script->author.c_str());

		ImGui::TableSetColumnIndex(3);
		ImGui::Text(script->champion.c_str());

		ImGui::TableSetColumnIndex(4);
		ImGui::Text(script->description.c_str());

		ImGui::TableSetColumnIndex(5);
		ImGui::Text("%d days ago", (int)((nowTimestamp - script->lastUpdate) / SECS_IN_DAY));

		ImGui::TableSetColumnIndex(6);
		int votes = script->numRatings;
		if (votes > 0) {
			int rating = (int)script->averageRating - 1;
			rating = (rating < 0) ? 0 : (rating >= NumRatings ? NumRatings - 1 : rating);
			ImGui::TextColored(ColorRatings[rating], Ratings[rating]);
			ImGui::SameLine();
			ImGui::Text("(%d)", votes);
		}
		else
			ImGui::TextColored(Color::GRAY, "No Votes");

		ImGui::TableSetColumnIndex(7);
		switch (script->type) {
		case RuntimeScript:
			ImGui::TextColored(Color::CYAN, "Runtime");
			break;
		case LibraryScript:
			ImGui::TextColored(Color::ORANGE, "Library");
			break;
		default:
			ImGui::TextColored(Color::RED, "Unknown");
		}
		ImGui::PopID();
	}

	ImGui::EndTable();
	DrawActions();
}

void ScriptRepository::DrawActions()
{
	ImGui::Separator();
	ImGui::TextColored(Color::PURPLE, "Actions");

	if (selectedScript == -1 || selectedScript >= sorted.size())
		return;

	auto entry = entries[sorted[selectedScript]];
	auto remote = entry->remote;
	auto local = entry->local;

	switch (entry->state) {
		
	case SE_STATE_UNINSTALLED:
		ImGui::TextColored(Color::GRAY, "You dont have this script. Press Install to get it");
		if (ImGui::Button("Install"))
			DownloadScriptAndInstall(remote);
		break;
	case SE_STATE_OUTDATED:
		ImGui::TextColored(Color::YELLOW, "This script is old. You can update it !");
		if (ImGui::Button("Update"))
			DownloadScriptAndInstall(remote);
		ImGui::SameLine();
	case SE_STATE_INSTALLED:
		if (ImGui::Button("Uninstall"))
			UninstallScript(local);
		ImGui::SameLine();
		HandleRating(local, remote);
		break;
	case SE_STATE_ONLY_LOCAL:
		ImGui::TextColored(Color::ORANGE, "This script is not on valkyrie servers. If you want to share it please submit it.");
		if (ImGui::Button("Delete"))
			UninstallScript(local);
		break;
	case SE_STATE_CORRUPTED:
		ImGui::TextColored(Color::RED, entry->error.c_str());
		if (ImGui::Button("Delete"))
			UninstallScript(local);
		if (remote != nullptr) {
			ImGui::SameLine();
			if (ImGui::Button("Try fix")) {
				UninstallScript(local);
				DownloadScriptAndInstall(remote);
			}
		}
		break;
	case SE_STATE_DOWNLOADING:
	case SE_STATE_WAITING:
		ImGui::TextColored(Color::CYAN, "This is script is being installed please wait");
		break;
	}

	if (local != nullptr && identity.name == local->author) {
		ImGui::SameLine();
		if (ImGui::Button("Submit"))
			SubmitScriptUpdate(local);
		DrawScriptEdit();

		if (entry->submission != nullptr) {
			switch (entry->submission->status) {
			case SUBMISSION_PENDING:
				ImGui::TextColored(Color::CYAN, "Your latest submission is pending approval.");
				break;
			case SUBMISSION_DENIED:
				ImGui::TextColored(Color::RED, "Your latest submission was denied. Reason: %s", entry->submission->denyReason.c_str());
				break;
			case SUBMISSION_APPROVED:
				ImGui::TextColored(Color::GREEN, "Your latest submission was approved");
				break;
			}
		}
	}
}

void ScriptRepository::DrawScriptEdit()
{
	bool changed = false;

	std::string idPrevious(idBuff);
	if (ImGui::InputText("Script Id", idBuff, SIZE_BUFF))
		changed = true;

	if (ImGui::InputText("Script Name", nameBuff, SIZE_BUFF))
		changed = true;

	if (ImGui::InputText("Champion Name", champBuff, SIZE_BUFF))
		changed = true;

	if (ImGui::InputText("Description", descBuff, SIZE_BUFF))
		changed = true;

	if (ImGui::Combo("Type", &type, "Runtime\0Library"))
		changed = true;

	if (DrawDependenciesSelect())
		changed = true;

	if (changed) {

		/// Check if new id overwrites existing one or new id is empty
		std::string idNew(idBuff);
		bool nameChanged = idNew != idPrevious;
		if (nameChanged && (idNew.size() == 0 || entries.find(idNew) != entries.end())) {
			strcpy_s(idBuff, idPrevious.c_str());
			return;
		}
		
		/// Remove the entry with old values create a new one with new values and rename file if id changed
		RemoveEntry(idPrevious, true, false);
		
		std::shared_ptr<ScriptInfo> info = std::shared_ptr<ScriptInfo>(new ScriptInfo());
		info->id = idNew;
		info->name = nameBuff;
		info->description = descBuff;
		info->champion = champBuff;
		info->author = identity.name;
		info->type = (ScriptType) type;
		info->dependencies = dependencies;
	
		AddEntry(info, true);
		if(nameChanged)
			MoveFileA(Paths::GetScriptPath(idPrevious).c_str(), Paths::GetScriptPath(idNew).c_str());
	}
}

bool ScriptRepository::DrawDependenciesSelect() {

	bool modified = false;
	if (ImGui::BeginListBox("Dependencies")) {
		for (int i = 0; i < dependencies.size(); ++i) {
			if (ImGui::Selectable(dependencies[i].c_str(), i == selectedDependency)) {
				selectedDependency = i;
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add"))
		ImGui::OpenPopup("DependencySelectPopup");
	if (selectedDependency > -1) {
		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			dependencies.erase(dependencies.begin() + selectedDependency);
			modified = true;
			selectedDependency -= 1;
		}
	}

	if (ImGui::BeginPopupModal("DependencySelectPopup")) {
		
		for (auto entry : entries) {
			auto local = entry.second->local;
			if (local != nullptr) {
				if (ImGui::Selectable(local->id.c_str())) {
					dependencies.push_back(local->id);
					modified = true;
					break;
				}
			}
		}

		ImGui::EndPopup();
	}

	return modified;
}

ImVec4 ScriptRepository::GetStateColor(ScriptEntryState state)
{
	switch (state) {
		case SE_STATE_DOWNLOADING:      return Color::CYAN;
		case SE_STATE_UNINSTALLED:      return Color::BROWN;
		case SE_STATE_INSTALLED:        return Color::NONE;
		case SE_STATE_WAITING:          return Color::CYAN;
		case SE_STATE_OUTDATED:         return Color::YELLOW;
		case SE_STATE_CORRUPTED:        return Color::RED;
		case SE_STATE_ONLY_LOCAL:       return Color::PURPLE;
	}
	return Color::NONE;
}

void ScriptRepository::SelectEntry(int selected)
{
	this->selectedScript = selected;
	auto local = entries[sorted[selected]]->local;
	if (local == nullptr)
		return;

	strcpy_s(idBuff, local->id.c_str());
	strcpy_s(nameBuff, local->name.c_str());
	strcpy_s(descBuff, local->description.c_str());
	strcpy_s(champBuff, local->champion.c_str());
	type = local->type;
	dependencies = local->dependencies;
}

void ScriptRepository::DownloadScriptAndInstall(std::shared_ptr<ScriptInfo> remote)
{
	taskPool->DispatchTask(
		remote->id,
		api->GetScriptCode(identity, remote->id),
		[remote, this](std::shared_ptr<AsyncTask> response) {
			std::string code = ((StringResultAsync*)response.get())->result.c_str();
			mtxEntries.lock();
			InstallScript(remote, code);
			mtxEntries.unlock();

			/// Install dependency scripts
			for (auto dependency : remote->dependencies) {
				auto find = entries.find(dependency);
				if (find != entries.end()) {
					if(find->second->local == nullptr && find->second->remote != nullptr)
						DownloadScriptAndInstall(find->second->remote);
				}
			}
		}
	);
}

void ScriptRepository::SubmitScriptUpdate(std::shared_ptr<ScriptInfo> script)
{
	/// Read code file
	std::ifstream stream(Paths::GetScriptPath(script->id));
	if (!stream.is_open())
		return;
	std::stringstream ss;
	ss << stream.rdbuf();
	std::string code = ss.str();

	taskPool->DispatchTask(
		std::string("SubmitScript"),
		api->SubmitScript(identity, *script, code),
		[this](std::shared_ptr<AsyncTask> response) {
			mtxEntries.lock();
			UpdateSubmissions(((ScriptSubmissionsResultAsync*)response.get())->submissions);
			mtxEntries.unlock();
		}
	);
}

void ScriptRepository::UpdateSubmissions(std::vector<std::shared_ptr<ScriptSubmission>>& submissions)
{
	for (auto& submission : submissions) {
		auto scriptId = submission->script->id;
		auto find = entries.find(scriptId);
		if (find == entries.end())
			continue;

		find->second->submission = submission;
	}
}

std::shared_ptr<ScriptEntry> ScriptRepository::InstallScript(std::shared_ptr<ScriptInfo> script, std::string & code)
{
	std::ofstream out(Paths::GetScriptPath(script->id));
	if (out.is_open())
		out << code;

	return AddEntry(script, true);
}

void ScriptRepository::UninstallScript(std::shared_ptr<ScriptInfo> script)
{
	DeleteFileA(Paths::GetScriptPath(script->id).c_str());
	RemoveEntry(script->id, true, false);
	selectedScript = -1;
}

void ScriptRepository::CreateLocalEntry()
{
	std::string newScriptId;
	int i = 0;
	while (true) {
		newScriptId.clear();
		newScriptId.append("New Script ");
		newScriptId.append(std::to_string(i));
		if (entries.find(newScriptId) == entries.end())
			break;
		i++;
	}

	std::shared_ptr<ScriptInfo> script = std::shared_ptr<ScriptInfo>(new ScriptInfo());
	script->id = newScriptId;
	script->name = "New Script";
	script->description = "Your description";
	script->champion = "all";
	script->author = identity.name;

	InstallScript(script, BaseCodeScript);

	for (size_t i = 0; i < sorted.size(); ++i) {
		if (sorted[i] == newScriptId)
			SelectEntry(i);
	}
}

void ScriptRepository::SortEntries()
{
	sorted.clear();

	auto searchLower = Strings::ToLower(searchStr);
	bool search = (searchLower.size() > 0);

	for (auto& pair : entries) {
		if (search) {
			auto info = (pair.second->remote != nullptr ? pair.second->remote : pair.second->local);

			/// Kinda inefficient but its just a local search fuck it
			if (info->champion.find(searchLower)               == std::string::npos && 
				Strings::ToLower(info->name).find(searchLower) == std::string::npos &&
				Strings::ToLower(info->id).find(searchLower)   == std::string::npos)
				continue;
		}
		sorted.push_back(pair.first);
	}

	std::sort(sorted.begin(), sorted.end(), comparator);
}

void ScriptRepository::UpdateState(std::shared_ptr<ScriptEntry>& entry)
{
	auto local = entry->local;
	auto remote = entry->remote;

	if (remote == nullptr) {
		auto path = Paths::GetScriptPath(entry->local->id);
		if (Paths::FileExists(path))
			entry->state = SE_STATE_ONLY_LOCAL;
		else {
			entry->state = SE_STATE_CORRUPTED;
			entry->error = "Script is missing from file system. Try reinstalling";
		}
		return;
	}

	if (local != nullptr) {

		for (auto dep : local->dependencies) {
			auto find = entries.find(dep);
			if (find == entries.end() || find->second->local == nullptr) {
				entry->state = SE_STATE_CORRUPTED;
				entry->error = Strings::Format("Scripts needs %s installed. Please install it otherwise script might not work. ", dep.c_str());
				return;
			}
		}

		if (taskPool->IsExecuting(local->id))
			entry->state = SE_STATE_DOWNLOADING;
		else if (taskPool->IsWaiting(local->id))
			entry->state = SE_STATE_WAITING;
		else if (std::abs(local->lastUpdate - remote->lastUpdate) > 1.0)
			entry->state = SE_STATE_OUTDATED;
		else {
			auto path = Paths::GetScriptPath(entry->local->id);
			if (Paths::FileExists(path))
				entry->state = SE_STATE_INSTALLED;
			else {
				entry->state = SE_STATE_CORRUPTED;
				entry->error = "Script is missing from file system. Try reinstalling";
			}
		}
	}
	else
		entry->state = SE_STATE_UNINSTALLED;
}

void ScriptRepository::UpdateInstalledScripts()
{
	for (auto pair : entries) {
		auto& entry = pair.second;
		if (entry->local != nullptr && entry->remote != nullptr && std::abs(entry->local->lastUpdate - entry->remote->lastUpdate) > 1.f)
			DownloadScriptAndInstall(entry->remote);
	}
}

void ScriptRepository::HandleRating(std::shared_ptr<ScriptInfo>& local, std::shared_ptr<ScriptInfo>& remote)
{
	if (ImGui::Button("Rate")) {
		ImGui::OpenPopup("RatePopup");
	}

	if (ImGui::BeginPopup("RatePopup")) {

		for (int i = 0; i < NumRatings; ++i) {
			if (ImGui::Button(Ratings[i])) {

				taskPool->DispatchTask(
					std::string("RateScript"),
					api->RateScript(identity, local->id, (float)(i + 1)),
					[this, local, remote](std::shared_ptr<AsyncTask> response) {
						mtxEntries.lock();
						auto resp = (ScriptRatingResultAsync*)response.get();
						local->averageRating = resp->newAverageRating;
						local->numRatings = resp->newNumRatings;
						remote->averageRating = resp->newAverageRating;
						remote->numRatings = resp->newNumRatings;
						mtxEntries.unlock();
					}
				);
			}
		}
		ImGui::EndPopup();
	}
}
