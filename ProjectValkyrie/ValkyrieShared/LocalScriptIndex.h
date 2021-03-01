#pragma once
#include <set>

#include "ScriptInfo.h"
#include "ScriptIndex.h"

class LocalScriptIndex: public ScriptIndex {

public:

	void LoadFromFile(const char* filePath);
	void SaveToFile(const char* filePath);
	void QueueInstall(ScriptInfo& script, std::string& scriptCode);
	void QueueUninstall(std::string id);

public:
	ScriptIndex*   remote;
	std::string    folderScripts;

protected:
	virtual int    GetAdditionalColumnCount();
	virtual void   SetupAdditionalColumns();
	virtual void   DrawAdditionalColumns(int lastColumn, std::shared_ptr<ScriptEntry> entry);
	virtual ScriptEntryStatus GetStatus(ScriptInfo& script);
};

class IndexQueuedInstall : public IndexQueuedOp {

public:
	             IndexQueuedInstall(ScriptInfo& entry, std::string scriptCode);
	virtual void Perform(ScriptIndex* index);

private:
	std::string   code;
	ScriptInfo&   entry;
};

class IndexQueuedUninstall : public IndexQueuedOp {

public:
	             IndexQueuedUninstall(std::string id);
	virtual void Perform(ScriptIndex* index);

private:
	std::string scriptId;
};