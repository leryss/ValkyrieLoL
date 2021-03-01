#include "ScriptIndex.h"
#include "Color.h"
#include <chrono>

using namespace std::chrono;

void ScriptIndex::Draw()
{
	static const float SECS_IN_DAY = 24.f * 60.f * 60.f;

	ImGui::BeginTable("Scripts", 7 + GetAdditionalColumnCount(), ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable);
	ImGui::TableSetupColumn("Id");
	ImGui::TableSetupColumn("Name");
	ImGui::TableSetupColumn("Author");
	ImGui::TableSetupColumn("Champion");
	ImGui::TableSetupColumn("Description");
	ImGui::TableSetupColumn("Last Update");
	ImGui::TableSetupColumn("Status");
	SetupAdditionalColumns();

	ImGui::TableHeadersRow();

	duration<float, std::milli> nowTimestamp = high_resolution_clock::now().time_since_epoch();
	for (size_t i = 0; i < sortable.size(); ++i) {

		ImGui::PushID(i);

		auto& id = sortable[i];
		auto& entry = index[id];
		auto& script = entry->script;

		entry->status = GetStatus(script);

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s.py", script.id.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::Text(script.name.c_str());

		ImGui::TableSetColumnIndex(2);
		ImGui::TextColored((script.author == "TeamValkyrie" ? Color::CYAN : Color::WHITE), script.author.c_str());

		ImGui::TableSetColumnIndex(3);
		ImGui::Text(script.champion.c_str());

		ImGui::TableSetColumnIndex(4);
		ImGui::Text(script.description.c_str());

		ImGui::TableSetColumnIndex(5);
		ImGui::Text("%d days ago", (int)((nowTimestamp.count() - script.lastUpdate) / SECS_IN_DAY));

		ImGui::TableSetColumnIndex(6);
		switch (entry->status) {
			case SES_DOWNLOADING:   ImGui::TextColored(Color::CYAN, "Downloading");    break;
			case SES_NOT_INSTALLED: ImGui::TextColored(Color::GRAY, "Not Installed");  break;
			case SES_INSTALLED:     ImGui::TextColored(Color::GREEN, "Installed");     break;
			case SES_WAITING:       ImGui::TextColored(Color::CYAN, "Waiting");        break;
			case SES_OUTDATED:      ImGui::TextColored(Color::YELLOW, "Outdated");     break;
		}

		DrawAdditionalColumns(7, entry);

		ImGui::PopID();
	}

	ImGui::EndTable();
}

bool ScriptIndex::HasEntry(std::string id)
{
	return index.find(id) != index.end();
}

std::shared_ptr<ScriptEntry> ScriptIndex::GetEntry(std::string id)
{
	auto find = index.find(id);
	if (find == index.end())
		return nullptr;

	return find->second;
}

void ScriptIndex::CreateEntry(std::shared_ptr<ScriptEntry> entry)
{
	index[entry->script.id] = entry;
	sortable.push_back(entry->script.id);
	dirty = true;
}

void ScriptIndex::CreateEntry(ScriptInfo& script)
{
	index[script.id] = std::shared_ptr<ScriptEntry>(new ScriptEntry(script));
	sortable.push_back(script.id);
	dirty = true;
}

void ScriptIndex::RemoveEntry(std::string id)
{
	auto find = index.find(id);
	if (find == index.end())
		return;
	index.erase(find);

	sortable.clear();
	for (auto& pair : index) {
		sortable.push_back(pair.first);
	}

	dirty = true;
}

void ScriptIndex::QueueAdd(ScriptInfo & script)
{
	queueMtx.lock();
	queueOps.push(new IndexQueuedAdd(script));
	queueMtx.unlock();
}

void ScriptIndex::QueueRemove(std::string id)
{
	queueMtx.lock();
	queueOps.push(new IndexQueuedDelete(id));
	queueMtx.unlock();
}

void ScriptIndex::QueueLoad(std::vector<ScriptInfo>& scripts)
{
	queueMtx.lock();
	queueOps.push(new IndexQueuedLoad(scripts));
	queueMtx.unlock();
}

void ScriptIndex::PerformQueued() {
	queueMtx.lock();
	while (!queueOps.empty()) {
		auto op = queueOps.front();
		queueOps.pop();
		op->Perform(this);
		delete op;
	}
	queueMtx.unlock();
}

IndexQueuedDelete::IndexQueuedDelete(std::string id)
	:scriptId(id)
{
}

void IndexQueuedDelete::Perform(ScriptIndex * index)
{
	index->RemoveEntry(scriptId);
}

IndexQueuedAdd::IndexQueuedAdd(ScriptInfo s)
	:script(s)
{
}

void IndexQueuedAdd::Perform(ScriptIndex * index)
{
	index->CreateEntry(script);
}

IndexQueuedLoad::IndexQueuedLoad(std::vector<ScriptInfo>& scripts) :
	scripts(scripts){

}

void IndexQueuedLoad::Perform(ScriptIndex* index) {
	for (auto& script : scripts) {
		index->CreateEntry(script);
	}
}

ScriptEntry::ScriptEntry(ScriptInfo& script)
	:script(script)
{
}
