#pragma once
#include "LoaderPanel.h"
#include "ScriptInfo.h"
#include "LocalScriptIndex.h"
#include "RemoteScriptIndex.h"

class UserPanel: public LoaderPanel {

public:
	             UserPanel();
	virtual void Draw(ValkyrieLoader& loader);

private:
	void         DrawHome();
	void         DrawScriptRepo();
	void         DrawScriptManager();

	void         LoadScriptIndicesIfNecessary();

public:
	std::string                 changeLog;
	bool                        autoInject;

private:
	std::string                 trackIdInjector       = "Injector";
	std::string                 trackIdUpdate         = "Update";
	std::string                 trackIdCheckVersion   = "CheckVersion";
	
	std::shared_ptr<AsyncTask>  injectorTask;

	/// Update related
	bool                        performUpdate  = true;
	bool                        updateComplete = false;

	/// Script market related
	bool                        loadLocalScripts = true;
	bool                        loadRemoteScripts = true;

	LocalScriptIndex            localScripts;
	RemoteScriptIndex           remoteScripts;
	ValkyrieLoader*             loader;
};