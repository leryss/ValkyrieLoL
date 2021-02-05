#include "Script.h"
#include "Logger.h"
#include "Strings.h"
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

bool Script::LoadInfo() {
	PyObject* dictName = PyUnicode_FromString("script_info");
	PyObject* dictAttr = PyObject_GetAttr(moduleObj, dictName);
	Py_DECREF(dictName);

	if (dictAttr == NULL) {
		error = std::string("No `script_info` dictionary found in script");
		return false;
	}

	dict d;
	try {
		d = dict(handle<>(dictAttr));

		author      = extract<std::string>(d.get("author"));
		description = extract<std::string>(d.get("description"));
		prettyName  = extract<std::string>(d.get("name"));
	}
	catch (error_already_set) {

		error = std::string("`script_info` dict must have the following strings `author`, `description`, `name`");
		return false;
	}

	try {
		targetChamp = extract<std::string>(d.get("target_champ"));
	}
	catch (error_already_set) {}

	return true;
}

bool Script::LoadFromFile(std::string & file)
{
	neverExecuted = false;
	loaded        = false;
	fileName      = file;
	error.clear();
	
	if (NULL != moduleObj) {
		Logger::LogAll("Reloading script %s", file.c_str());
		moduleObj = PyImport_ReloadModule(moduleObj);
	}
	else {
		Logger::LogAll("Loading script %s", file.c_str());
		moduleObj = PyImport_ImportModule(file.c_str());
	}

	if (NULL == moduleObj) {
		Logger::LogAll("Error loading %s", file.c_str());

		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);

		error.append("Failed to load: ");
		error.append(extract<std::string>(PyObject_Str(pvalue)));
	}
	else {
		if (LoadInfo() &&
			LoadFunc(&functions[ScriptFunction::ON_LOOP], "valkyrie_exec") &&
			LoadFunc(&functions[ScriptFunction::ON_MENU], "valkyrie_menu") &&
			LoadFunc(&functions[ScriptFunction::ON_LOAD], "valkyrie_on_load") &&
			LoadFunc(&functions[ScriptFunction::ON_SAVE], "valkyrie_on_save")) {

			config.SetSaveInterval(100);
			config.SetConfigFile(fileName.c_str());
			config.Load();
			neverExecuted = true;
			loaded        = true;
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
		call<void>(functions[func], object(boost::ref(ctx)));
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
