#pragma once

#include "ConfigSet.h"
#include "PyExecutionContext.h"
#include "InputController.h"
#include "Benchmark.h"
#include "ScriptInfo.h"

#include <string>
#include <boost/python.hpp>

using namespace boost::python;

enum ScriptFunction {
	ON_LOOP = 0,
	ON_MENU = 1,
	ON_LOAD = 2,
	ON_SAVE = 3
};

enum ScriptState {
	ScriptReady    = 1,
	ScriptFailed   = 2,
	ScriptDisabled = 3
};

class Script {
public:
	                   Script();
				       ~Script();

	/// Loads the script from the scripts folder using the script index info provided
	bool               Load(std::shared_ptr<ScriptInfo> info);

	/// Executes a specific script function
	void               Execute(PyExecutionContext& ctx, ScriptFunction func);

	/// Enable disable script
	void               SetEnabled(bool enabled);

	bool               IsEnabled();

	bool               HasFunction(ScriptFunction func);

	/// Exctracts the python error from CPython
	static std::string GetPyError();
				 
public:

	bool               neverExecuted;
	ScriptState        state;
	
	std::string        error;
	std::shared_ptr<ScriptInfo> info;

	ConfigSet          config;
	InputController    input;
	BenchmarkTiming    executionTimes[4];

	object             context;

private:		 

	bool         enabled;

	/// Attempts to load function with name funcName from the script code
	bool         LoadFunc(PyObject** loadInto, const char* funcName);

	/// Sets the state to errored and fills the error string
	void         SetError(std::string reason, std::string error);

	/// Clears the errored state and the error string
	void         ClearError();

	void         UpdateState();

	const int    NUM_FUNCTIONS = 4;
	PyObject*    moduleObj;
	PyObject*    functions[20];
};