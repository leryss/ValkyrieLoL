#include "RemoteScriptIndex.h"
#include "LocalScriptIndex.h"

void RemoteScriptIndex::Load(const IdentityInfo& identity)
{
	this->identity = identity;

	index.clear();
	sortable.clear();

	taskPool->DispatchTask(
		trackIdGetScripts,
		api->GetScriptList(identity),
		[this](std::shared_ptr<AsyncTask> response) {
			QueueLoad(((ScriptListAsync*)response.get())->scripts);
		}
	);
}

int RemoteScriptIndex::GetAdditionalColumnCount()
{
	return 2;
}

void RemoteScriptIndex::SetupAdditionalColumns()
{
	ImGui::TableSetupColumn("Downloads");
	ImGui::TableSetupColumn("Actions");
}

void RemoteScriptIndex::DrawAdditionalColumns(int lastColumn, std::shared_ptr<ScriptEntry> entry)
{
	/// Download column
	ImGui::TableSetColumnIndex(lastColumn);
	ImGui::Text("%d", entry->script.downloads);

	ImGui::TableSetColumnIndex(lastColumn + 1);
	switch (entry->status) {

		case SES_OUTDATED:
			if (ImGui::Button("Update"))
				SendUpdateRequest(entry->script);
			ImGui::SameLine();
		case SES_INSTALLED:
			if (ImGui::Button("Uninstall"))
				((LocalScriptIndex*)local)->QueueUninstall(entry->script.id);
			break;
		case SES_NOT_INSTALLED:
			if (ImGui::Button("Install")) {
				SendUpdateRequest(entry->script);
			}
			break;
	}
}

ScriptEntryStatus RemoteScriptIndex::GetStatus(ScriptInfo& script)
{
	auto localEntry = local->GetEntry(script.id);

	if (localEntry != nullptr) {

		if (script.lastUpdate != localEntry->script.lastUpdate)
			return SES_OUTDATED;

		if (taskPool->IsExecuting(script.id)) {
			return SES_DOWNLOADING;
		}
		else if (taskPool->IsWaiting(script.id)) {
			return SES_WAITING;
		}
		else
			return SES_INSTALLED;
	}
	else
		return SES_NOT_INSTALLED;
}

void RemoteScriptIndex::SendUpdateRequest(ScriptInfo& script)
{
	taskPool->DispatchTask(script.id, api->GetScriptCode(identity, script.id),
		[this, &script](std::shared_ptr<AsyncTask> response) {
			LocalScriptIndex* idx = (LocalScriptIndex*)local;
			StringResultAsync* resp = (StringResultAsync*)(response.get());
			std::string code = std::string(resp->result.c_str());

			idx->QueueInstall(script, code);
	});
}

void RemoteScriptIndex::Draw()
{
	if (taskPool->IsExecuting(trackIdGetScripts)) {
		ImGui::TextColored(Color::YELLOW, "Retrieving scripts...");
		return;
	}

	ScriptIndex::Draw();
}