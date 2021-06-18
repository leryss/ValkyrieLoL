#include "Script.h"
#include "Logger.h"
#include "Strings.h"
#include "Paths.h"
#include <boost/python/detail/convertible.hpp>

std::string Script::GetPyError()
{
	PyObject *exc, *val, *tb;
	PyErr_Fetch(&exc, &val, &tb);
	PyErr_NormalizeException(&exc, &val, &tb);

	object main = import("__main__");
	exec("from traceback import format_exception", main.attr("__dict__"));
	PyObject* formatFunc = object(main.attr("format_exception")).ptr();
	
	auto hExc    = handle<>(exc);
	auto hVal    = handle<>(val);
	auto hTb     = handle<>(tb);
	list errList = call<list>(formatFunc, hExc, hVal, hTb);
	str  errStr  = str("");
	errStr       = errStr.join(errList);

	return std::string(extract<const char*>(errStr));
}

Script::Script()
{
	neverExecuted = false;

	moduleObj = NULL;
	functions[ScriptFunction::ON_LOOP] = NULL;
	functions[ScriptFunction::ON_LOAD] = NULL;
	functions[ScriptFunction::ON_MENU] = NULL;
	functions[ScriptFunction::ON_SAVE] = NULL;
}

Script::~Script()
{
	if (moduleObj != NULL)
		Py_DECREF(moduleObj);
	for (int i = 0; i < NUM_FUNCTIONS; ++i) {
		if (functions[i] != NULL)
			Py_DECREF(functions[i]);
	}
}

bool Script::LoadFunc(PyObject** loadInto, const char* funcName) {
	if (*loadInto != NULL)
		Py_DECREF(*loadInto);

	PyObject* pyFuncName = PyUnicode_FromString(funcName);
	*loadInto = PyObject_GetAttr(moduleObj, pyFuncName);
	Py_DECREF(pyFuncName);

	if (*loadInto == NULL) {
		return false;
	}
	return true;
}

void Script::SetError(std::string reason, std::string err)
{
	error.clear();
	error.append(reason);
	error.append("\n");
	error.append(err);
	UpdateState();
}

void Script::ClearError()
{
	error.clear();
	UpdateState();
}

void Script::UpdateState()
{
	if (!error.empty()) {
		state = ScriptFailed;
	}
	else {
		state = enabled ? ScriptReady : ScriptDisabled;
	}

}

bool Script::Load(std::shared_ptr<ScriptInfo> info)
{
	ClearError();
	neverExecuted = true;
	this->info    = info;
	
	if (NULL != moduleObj) {
		Logger::Info("Reloading script %s", info->id.c_str());
		moduleObj = PyImport_ReloadModule(moduleObj);
	}
	else {
		Logger::Info("Loading script %s", info->id.c_str());
		moduleObj = PyImport_ImportModule(info->id.c_str());
	}

	if (NULL == moduleObj) {
		Logger::Error("Error loading %s", info->id.c_str());

		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);

		SetError("Failed to load", extract<std::string>(PyObject_Str(pvalue)));
	}
	else {
		LoadFunc(&functions[ScriptFunction::ON_LOOP], "valkyrie_exec");
		LoadFunc(&functions[ScriptFunction::ON_MENU], "valkyrie_menu");
		LoadFunc(&functions[ScriptFunction::ON_LOAD], "valkyrie_on_load");
		LoadFunc(&functions[ScriptFunction::ON_SAVE], "valkyrie_on_save");


		auto configPath = Paths::Configs;
		configPath.append("\\");
		configPath.append(info->id.c_str());
		config.SetSaveInterval(100);
		config.SetConfigFile(configPath);
		config.Load();

		SetEnabled(config.GetBool("__enabled", true));

		context = import(info->id.c_str()).attr("__dict__");
		return true;
	}

	return false;
}

void Script::Execute(PyExecutionContext& ctx, ScriptFunction func)
{
	if (state != ScriptReady || functions[func] == NULL)
		return;

	try {
		neverExecuted = false;

		executionTimes[func].Start();
		call<void>(functions[func], ctx.selfPy);
		executionTimes[func].End();
	}
	catch (error_already_set) {
		SetError("Execution failure", GetPyError());
	}
	catch (std::exception& e) {
		SetError("Unknown error", e.what());
	}
}

void Script::SetEnabled(bool enabled)
{
	if (enabled == this->enabled)
		return;

	this->enabled = enabled;
	config.SetBool("__enabled", enabled);
	config.Save();
	UpdateState();
}

bool Script::IsEnabled()
{
	return enabled;
}

bool Script::HasFunction(ScriptFunction func)
{
	return functions[func] != NULL;
}
