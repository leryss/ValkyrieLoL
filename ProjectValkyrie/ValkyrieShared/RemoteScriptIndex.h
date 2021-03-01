#pragma once

#include "ScriptIndex.h"
#include "ValkyrieAPI.h"
#include "AsyncTaskPool.h"

class RemoteScriptIndex : public ScriptIndex {

public:

	void           Load(const IdentityInfo& identity);
	virtual void   Draw() override;

protected:
	virtual int    GetAdditionalColumnCount();
	virtual void   SetupAdditionalColumns();
	virtual void   DrawAdditionalColumns(int lastColumn, std::shared_ptr<ScriptEntry> entry);
	virtual ScriptEntryStatus GetStatus(ScriptInfo& script);

	void           SendUpdateRequest(ScriptInfo& script);

public:
	ScriptIndex*   local;
	IdentityInfo   identity;

private:
	std::string    trackIdGetScripts = "GetScriptList";

	AsyncTaskPool* taskPool = AsyncTaskPool::Get();
	ValkyrieAPI*   api      = ValkyrieAPI::Get();
};