#include "LocalScriptIndex.h"
#include "Color.h"
#include "Strings.h"

#include "imgui/imgui.h"

#include <fstream>
#include <aws/core/utils/json/JsonSerializer.h>

void LocalScriptIndex::LoadFromFile(const char * fileName)
{
	std::ifstream stream;
	stream.open(fileName);

	if (!stream.is_open())
		return;

	sortable.clear();
	index.clear();

	JsonValue json(stream);
	
	auto jscripts = json.View().GetArray("installed");
	for (size_t i = 0; i < jscripts.GetLength(); ++i) {
		auto& jinfo = jscripts.GetItem(i);

		auto id = std::string(jinfo.GetString("id").c_str());
		auto script = ScriptInfo::FromJsonView(jinfo);
		index[id] = std::shared_ptr<ScriptEntry>(new ScriptEntry(script));
		sortable.push_back(id);
	}
}

void LocalScriptIndex::SaveToFile(const char * filePath)
{
	std::ofstream stream;
	stream.open(filePath, std::ofstream::out);

	if (!stream.is_open()) {
		throw std::exception(Strings::Format("Failed to open index file for reading: %s", filePath).c_str());
	}
	
	JsonValue json;
	Aws::Utils::Array<JsonValue> jscripts(index.size());

	size_t i = 0;
	for (auto& pair : index) {
		jscripts[i] = pair.second->script.ToJsonValue();
		i++;
	}

	json.WithArray("installed", jscripts);
	stream << json.View().WriteReadable();

	dirty = false;
}

void LocalScriptIndex::QueueInstall(ScriptInfo & script, std::string & scriptCode)
{
	queueMtx.lock();
	queueOps.push(new IndexQueuedInstall(script, scriptCode));
	queueMtx.unlock();
}

void LocalScriptIndex::QueueUninstall(std::string id)
{
	queueMtx.lock();
	queueOps.push(new IndexQueuedUninstall(id));
	queueMtx.unlock();
}

int LocalScriptIndex::GetAdditionalColumnCount()
{
	return 1;
}

void LocalScriptIndex::SetupAdditionalColumns()
{
	ImGui::TableSetupColumn("Actions");
}

void LocalScriptIndex::DrawAdditionalColumns(int lastColumn, std::shared_ptr<ScriptEntry> entry)
{
	ImGui::TableSetColumnIndex(lastColumn);
}

ScriptEntryStatus LocalScriptIndex::GetStatus(ScriptInfo & script)
{
	auto localEntry = remote->GetEntry(script.id);

	if (localEntry != nullptr) {

		if (script.lastUpdate != localEntry->script.lastUpdate)
			return SES_OUTDATED;
		else
			return SES_INSTALLED;
	}
	else
		return SES_NOT_INSTALLED;
}

IndexQueuedInstall::IndexQueuedInstall(ScriptInfo& entry, std::string scriptCode)
	:
	code(scriptCode),
	entry(entry)
{
}

void IndexQueuedInstall::Perform(ScriptIndex * index)
{
	LocalScriptIndex* local = (LocalScriptIndex*)index;

	std::string filePath = Paths::Scripts + "\\" + entry.id + ".py";
	std::ofstream file(filePath);
	if (!file.is_open())
		throw std::exception(Strings::Format("Can't open file for writing: %s", filePath.c_str()).c_str());

	file << code;
	index->CreateEntry(entry);
}

IndexQueuedUninstall::IndexQueuedUninstall(std::string id)
	:scriptId(id)
{
}

void IndexQueuedUninstall::Perform(ScriptIndex * index)
{
	LocalScriptIndex* local = (LocalScriptIndex*)index;

	std::string filePath = Paths::Scripts + "\\" + scriptId + ".py";
	if (!DeleteFileA(filePath.c_str()))
		throw std::exception(Strings::Format("Cannot delete file: %s", filePath.c_str()).c_str());

	index->RemoveEntry(scriptId);
}
