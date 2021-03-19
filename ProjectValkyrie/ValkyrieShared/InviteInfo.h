#pragma once
#include <string>
#include "UserInfo.h"

enum InviteMode {
	INVITE_MODE_CREATE,
	INVITE_MODE_EXTEND
};

class InviteInfo {

public:
	InviteMode  mode;
	UserLevel   level;
	float       days;
	std::string code;

	static std::shared_ptr<InviteInfo> FromJsonView(const JsonView& view);
	JsonValue ToJsonValue() const;
};