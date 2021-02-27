#pragma once
#include <string>
#include "HardwareInfo.h"

enum UserLevel {
	USER_LEVEL_NORMAL,
	USER_LEVEL_TESTER,
	USER_LEVEL_ADMIN,
	USER_LEVEL_SUPER_ADMIN
};

class UserInfo {

public:
	std::string  name;
	bool         locked;
	bool         resetHardware;
	HardwareInfo hardware;
	std::string  discord;
	float        expiry;
	float        level;

	static UserInfo FromJsonView(const JsonView& view);
	JsonValue ToJsonValue() const;
};