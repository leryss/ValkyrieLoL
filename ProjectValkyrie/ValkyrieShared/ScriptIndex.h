#pragma once
#include <map>
#include <queue>
#include <mutex>

#include "Paths.h"
#include "ScriptInfo.h"
#include "imgui/imgui.h"

class ScriptIndex;
class IndexQueuedOp {

public:
	virtual void Perform(ScriptIndex* index) = 0;
};

class IndexQueuedDelete : public IndexQueuedOp {

public:
	IndexQueuedDelete(std::string id);
	virtual void Perform(ScriptIndex* index);
private:
	std::string  scriptId;
};

class IndexQueuedAdd : public  IndexQueuedOp {

public:
	IndexQueuedAdd(ScriptInfo script);
	virtual void Perform(ScriptIndex* index);
private:
	ScriptInfo   script;
};

class IndexQueuedLoad : public IndexQueuedOp {

public:
	IndexQueuedLoad(std::vector<ScriptInfo>& scripts);
	virtual void Perform(ScriptIndex* index);
private:
	std::vector<ScriptInfo>&   scripts;
};

enum ScriptEntryStatus {

	SES_OUTDATED,
	SES_NOT_INSTALLED,
	SES_DOWNLOADING,
	SES_WAITING,
	SES_INSTALLED
};

class ScriptEntry {

public:
	ScriptEntry(ScriptInfo& script);

public:
	ScriptInfo        script;
	ScriptEntryStatus status;
};

class ScriptIndex {

public:
	virtual void Draw();
	
	bool                          HasEntry(std::string id);
	std::shared_ptr<ScriptEntry>  GetEntry(std::string id);
	void                          CreateEntry(std::shared_ptr<ScriptEntry> entry);
	void                          CreateEntry(ScriptInfo& script);
	void                          RemoveEntry(std::string id);
				                  
	void                          QueueAdd(ScriptInfo& script);
	void                          QueueRemove(std::string id);
	void                          QueueLoad(std::vector<ScriptInfo>& scripts);
	void                          PerformQueued();

protected:

	virtual int                   GetAdditionalColumnCount() = 0;
	virtual void                  SetupAdditionalColumns() = 0;
	virtual void                  DrawAdditionalColumns(int lastColumn, std::shared_ptr<ScriptEntry> entry) = 0;
	virtual ScriptEntryStatus     GetStatus(ScriptInfo& script) = 0;
				                  
public:			                  
	bool                          dirty = false;

protected:
	std::map<std::string, std::shared_ptr<ScriptEntry>>  index;
	std::vector<std::string>           sortable;

	std::mutex                         queueMtx;
	std::queue<IndexQueuedOp*>         queueOps;
};