#pragma once
#include <string>
#include "ScriptInfo.h"

class ScriptSubmission {

public:
	std::string scriptId;
	ScriptInfo  scriptInfo;
	std::string scriptCode;

	static ScriptSubmission   FromJsonView(const JsonView& json);
	JsonValue                 ToJsonValue() const;
};