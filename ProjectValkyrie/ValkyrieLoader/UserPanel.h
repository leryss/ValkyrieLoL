#pragma once
#include "LoaderPanel.h"
#include "ScriptInfo.h"
#include "ScriptRepository.h"

class UserPanel: public LoaderPanel {

public:
	             UserPanel();
	virtual void Draw(ValkyrieLoader& loader);

private:
	void         DrawHome();
	void         DrawScriptRepo();
	void         ReadChangeLog();

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
	bool                        loadScriptRepo = true;

	ScriptRepository            scriptRepo;
	ValkyrieLoader*             loader;
};