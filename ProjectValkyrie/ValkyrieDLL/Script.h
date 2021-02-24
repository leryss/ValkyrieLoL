#pragma once

#include "ConfigSet.h"
#include "PyExecutionContext.h"
#include "InputController.h"
#include "Benchmark.h"

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
	bool               LoadFromFile(std::string& filePath);
	void               Execute(PyExecutionContext& ctx, ScriptFunction func);

	static std::string GetPyError();
				 
public:
	bool               loaded;
	bool               neverExecuted;
				       
	std::string        error;
	std::string        fileName;
	std::string        prettyName;
	std::string        author;
	std::string        description;
	std::string        targetChamp;
	std::string        icon;

	ConfigSet          config;
	InputController    input;
	BenchmarkTiming    executionTimes[4];

private:		 
				 
	bool         LoadFunc(PyObject** loadInto, const char* funcName);
	bool         LoadInfo();

	PyObject*    moduleObj;
	PyObject*    functions[4];
};