#include "ScriptSubmission.h"

std::shared_ptr<ScriptSubmission> ScriptSubmission::FromJsonView(const JsonView & json)
{
	std::shared_ptr<ScriptSubmission> s = std::shared_ptr<ScriptSubmission>(new ScriptSubmission());
	s->script     = ScriptInfo::FromJsonView(json.GetObject("script"));
	s->status     = (SubmissionStatus)json.GetInteger("status");
	s->denyReason = json.GetString("deny_reason").c_str();

	return s;
}

JsonValue ScriptSubmission::ToJsonValue() const
{
	JsonValue j;

	j.WithObject("script", script->ToJsonValue());
	j.WithInteger("status", status);
	j.WithString("deny_reason", denyReason.c_str());

	return j;
}
