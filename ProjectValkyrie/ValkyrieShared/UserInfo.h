#pragma once
#include <string>
#include "HardwareInfo.h"

class UserInfo {

public:
	std::string  name;
	bool         locked;
	HardwareInfo hardware;
	std::string  discord;
	float        expiry;
	float        level;

	static UserInfo FromJsonView(const JsonView& view);
};