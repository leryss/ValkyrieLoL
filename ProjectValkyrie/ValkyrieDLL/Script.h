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

class Script {
public:
	                   Script();
				       ~Script();
	bool               Load(std::shared_ptr<ScriptInfo> info);
	void               Execute(PyExecutionContext& ctx, ScriptFunction func);

	static std::string GetPyError();
				 
public:
	bool               loaded;
	bool               neverExecuted;
				       
	std::string        error;
	std::shared_ptr<ScriptInfo> info;

	ConfigSet          config;
	InputController    input;
	BenchmarkTiming    executionTimes[4];

private:		 
	bool         LoadFunc(PyObject** loadInto, const char* funcName);

	PyObject*    moduleObj;
	PyObject*    functions[4];
};