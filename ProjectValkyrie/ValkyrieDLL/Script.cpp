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
	loaded = false;
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
	for (int i = 0; i < 4; ++i) {
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
		error = Strings::Format("Failed to load function %s", funcName);
		return false;
	}
	return true;
}

bool Script::Load(std::shared_ptr<ScriptInfo> info)
{
	neverExecuted = false;
	loaded        = false;
	this->info    = info;
	error.clear();
	
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

		error.append("Failed to load: ");
		error.append(extract<std::string>(PyObject_Str(pvalue)));
	}
	else {
		if (LoadFunc(&functions[ScriptFunction::ON_LOOP], "valkyrie_exec") &&
			LoadFunc(&functions[ScriptFunction::ON_MENU], "valkyrie_menu") &&
			LoadFunc(&functions[ScriptFunction::ON_LOAD], "valkyrie_on_load") &&
			LoadFunc(&functions[ScriptFunction::ON_SAVE], "valkyrie_on_save")) {

			auto configPath = Paths::Configs;
			configPath.append("\\");
			configPath.append(info->id.c_str());
			config.SetSaveInterval(100);
			config.SetConfigFile(configPath);
			config.Load();

			neverExecuted = true;
			loaded        = true;

			context = import(info->id.c_str()).attr("__dict__");
			return true;
		}
	}

	return false;
}

void Script::Execute(PyExecutionContext& ctx, ScriptFunction func)
{
	if (!error.empty())
		return;

	try {
		input.UpdateIssuedOperations();
		neverExecuted = false;

		executionTimes[func].Start();
		call<void>(functions[func], ctx.selfPy);
		executionTimes[func].End();
	}
	catch (error_already_set) {
		error.clear();
		error.append(GetPyError());
	}
	catch (std::exception& e) {
		error.clear();
		error.append(e.what());
	}
}
