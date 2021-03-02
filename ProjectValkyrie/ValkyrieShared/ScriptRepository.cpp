#include "ScriptRepository.h"
#include "Strings.h"
#include "Paths.h"

#include <chrono>
#include <fstream>

using namespace std::chrono;

std::string ScriptRepository::BaseCodeScript = ""
"from valkyrie import *			 \n"
"								 \n"
"def valkyrie_menu(ctx) :		 \n"
"	ui = ctx.ui					 \n"
"								 \n"
"def valkyrie_on_load(ctx) :	 \n"
"	cfg = ctx.cfg				 \n"
"	pass						 \n"
"								 \n"
"def valkyrie_on_save(ctx) :	 \n"
"	cfg = ctx.cfg				 \n"
"	pass						 \n"
"								 \n"
"def valkyrie_exec(ctx) :	     \n"
"	pass						 \n"
"								 \n"
"";

void ScriptRepository::LoadLocalEntries(std::string path)
{
	mtxEntries.lock();

	std::ifstream stream;
	stream.open(path);

	if (!stream.is_open())
		return;

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
			mtxEntries.lock();
			auto scripts = ((ScriptListAsync*)response.get())->scripts;
			for (auto& script : scripts)
				AddEntry(script, false);
			SortEntries();
			mtxEntries.unlock();
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
			DrawTable(true);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Online")) {
			DrawTable(false);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	
	mtxEntries.unlock();
}

void ScriptRepository::AddEntry(ScriptInfo & script, bool isLocal)
{
	auto find = entries.find(script.id);
	std::shared_ptr<ScriptEntry> entry;
	if (find != entries.end()) {
		entry = find->second;
	}
	else { 
		sorted.push_back(script.id);
		entry = std::shared_ptr<ScriptEntry>(new ScriptEntry);
		entries[script.id] = entry;
	}

	if (isLocal) {
		entry->local = std::shared_ptr<ScriptInfo>(new ScriptInfo(script));
		localsUnsaved = true;
	}
	else
		entry->remote = std::shared_ptr<ScriptInfo>(new ScriptInfo(script));
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

	ImGui::TextColored(Color::PURPLE, "Scripts");
	ImGui::BeginTable("TableScripts", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable);
	ImGui::TableSetupColumn("Id");
	ImGui::TableSetupColumn("Name");
	ImGui::TableSetupColumn("Author");
	ImGui::TableSetupColumn("Champion");
	ImGui::TableSetupColumn("Description");
	ImGui::TableSetupColumn("Last Update");
	ImGui::TableSetupColumn("Status");

	ImGui::TableHeadersRow();

	duration<float, std::milli> nowTimestamp = high_resolution_clock::now().time_since_epoch();
	for (size_t i = 0; i < sorted.size(); ++i) {

		auto& id = sorted[i];
		auto& entry = entries[id];
		auto& script = (showLocal ? entry->local : entry->remote);
		if (script == nullptr)
			continue;

		UpdateState(entry);

		ImGui::PushID(i);
		ImGui::TableNextRow();

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
		ImGui::Text("%d days ago", (int)((nowTimestamp.count() - script->lastUpdate) / SECS_IN_DAY));

		ImGui::TableSetColumnIndex(6);
		switch (entry->state) {
			case SE_STATE_DOWNLOADING:   ImGui::TextColored(Color::CYAN,   "Downloading");     break;
			case SE_STATE_UNINSTALLED:   ImGui::TextColored(Color::GRAY,   "Not Installed");   break;
			case SE_STATE_INSTALLED:     ImGui::TextColored(Color::GREEN,  "Installed");       break;
			case SE_STATE_WAITING:       ImGui::TextColored(Color::CYAN,   "Waiting");         break;
			case SE_STATE_OUTDATED:      ImGui::TextColored(Color::YELLOW, "Outdated");        break;
			case SE_STATE_CORRUPTED:     ImGui::TextColored(Color::RED,    "Corrupted");       break;
			case SE_STATE_ONLY_LOCAL:    ImGui::TextColored(Color::YELLOW, "Local Only");      break;
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
	if (selectedScript == -1)
		return;

	auto entry = entries[sorted[selectedScript]];
	auto remote = entry->remote;
	auto local = entry->local;

	switch (entry->state) {
		
	case SE_STATE_UNINSTALLED:
		if (ImGui::Button("Install"))
			DownloadScriptAndInstall(*remote);
		break;
	case SE_STATE_OUTDATED:
		if (ImGui::Button("Update"))
			DownloadScriptAndInstall(*remote);
		ImGui::SameLine();
	case SE_STATE_INSTALLED:
		if (ImGui::Button("Uninstall"))
			UninstallScript(*local);
		break;
	case SE_STATE_ONLY_LOCAL:
	case SE_STATE_CORRUPTED:
		if (ImGui::Button("Delete"))
			UninstallScript(*local);
		break;
	}

	if (local != nullptr && identity.name == local->author) {
		ImGui::SameLine();
		if (ImGui::Button("Submit")) {}
		DrawScriptEdit();
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
		
		ScriptInfo info;
		info.id = idNew;
		info.name = nameBuff;
		info.description = descBuff;
		info.champion = champBuff;
		info.author = identity.name;
	
		AddEntry(info, true);
		if(nameChanged)
			MoveFileA(Paths::GetScriptPath(idPrevious).c_str(), Paths::GetScriptPath(idNew).c_str());
	}
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
}

void ScriptRepository::DownloadScriptAndInstall(ScriptInfo& remote)
{
	taskPool->DispatchTask(
		remote.id,
		api->GetScriptCode(identity, remote.id),
		[&remote, this](std::shared_ptr<AsyncTask> response) {
			std::string code = ((StringResultAsync*)response.get())->result.c_str();
			mtxEntries.lock();
			InstallScript(remote, code);
			mtxEntries.unlock();
		}
	);
}

void ScriptRepository::InstallScript(ScriptInfo & script, std::string & code)
{
	std::ofstream out(Paths::GetScriptPath(script.id));
	if (out.is_open())
		out << code;

	AddEntry(script, true);
}

void ScriptRepository::UninstallScript(ScriptInfo& script)
{
	DeleteFileA(Paths::GetScriptPath(script.id).c_str());
	RemoveEntry(script.id, true, false);
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

	ScriptInfo script;
	script.id = newScriptId;
	script.name = "New Script";
	script.description = "Your description";
	script.champion = "all";
	script.author = identity.name;

	InstallScript(script, BaseCodeScript);

	for (size_t i = 0; i < sorted.size(); ++i) {
		if (sorted[i] == newScriptId)
			SelectEntry(i);
	}
}

void ScriptRepository::SortEntries()
{
	sorted.clear();
	for (auto& pair : entries) {
		sorted.push_back(pair.first);
	}
}

void ScriptRepository::UpdateState(std::shared_ptr<ScriptEntry>& entry)
{
	auto local = entry->local;
	auto remote = entry->remote;

	if (remote == nullptr) {
		auto path = Paths::GetScriptPath(entry->local->id);
		if (Paths::FileExists(path))
			entry->state = SE_STATE_ONLY_LOCAL;
		else
			entry->state = SE_STATE_CORRUPTED;
	}
	else if (local != nullptr) {

		if (local->lastUpdate != remote->lastUpdate)
			entry->state = SE_STATE_OUTDATED;

		if (taskPool->IsExecuting(local->id)) {
			entry->state = SE_STATE_DOWNLOADING;
		}
		else if (taskPool->IsWaiting(local->id))
			entry->state = SE_STATE_WAITING;
		else {
			auto path = Paths::GetScriptPath(entry->local->id);
			if (Paths::FileExists(path))
				entry->state = SE_STATE_INSTALLED;
			else
				entry->state = SE_STATE_CORRUPTED;
		}
	}
	else
		entry->state = SE_STATE_UNINSTALLED;
}