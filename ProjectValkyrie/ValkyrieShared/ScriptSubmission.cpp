#include "ScriptSubmission.h"

ScriptSubmission ScriptSubmission::FromJsonView(const JsonView & json)
{
	ScriptSubmission submission;

	submission.scriptId   = json.GetString("script_id").c_str();
	submission.scriptCode = json.GetString("script_code").c_str();
	submission.scriptInfo = ScriptInfo::FromJsonView(json.GetObject("script_info"));

	return submission;
}

JsonValue ScriptSubmission::ToJsonValue() const
{
	JsonValue json;

	json.WithString("script_id", scriptId.c_str());
	json.WithObject("script_info", scriptInfo.ToJsonValue());
	json.WithString("script_code", scriptCode.c_str());

	return json;
}
