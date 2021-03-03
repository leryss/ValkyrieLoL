#pragma once
#include <string>
#include "ScriptInfo.h"

enum SubmissionStatus {
	SUBMISSION_PENDING = 0,
	SUBMISSION_APPROVED = 1,
	SUBMISSION_DENIED = 2
};

class ScriptSubmission {

public:
	std::shared_ptr<ScriptInfo> script;
	SubmissionStatus            status;
	std::string                 denyReason;

	static std::shared_ptr<ScriptSubmission> FromJsonView(const JsonView& json);
	JsonValue                 ToJsonValue() const;
};