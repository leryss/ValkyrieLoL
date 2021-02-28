#pragma once
#include "LoaderPanel.h"
#include "ScriptInfo.h"

class UserPanel: public LoaderPanel {

public:
	virtual void Draw(ValkyrieLoader& loader);

private:
	void         DrawHome();
	void         DrawScriptRepo();

public:
	std::string                 changeLog;
	bool                        autoInject;

private:
	std::string                 trackIdInjector       = "Injector";
	std::string                 trackIdUpdate         = "Update";
	std::string                 trackIdCheckVersion   = "CheckVersion";
	std::string                 trackIdGetScripts     = "GetScriptList";

	std::shared_ptr<AsyncTask>  injectorTask;

	bool                        performUpdate  = true;
	bool                        updateComplete = false;

	bool                        retrieveScripts = true;
	std::vector<ScriptInfo>     scripts;

	ValkyrieLoader*             loader;
};