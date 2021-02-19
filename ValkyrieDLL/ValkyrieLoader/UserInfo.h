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

	static UserInfo FromJsonView(const JsonView& view) {
		UserInfo user;

		#undef GetObject // Thank you Bill Gates

		user.name     = view.GetString("name").c_str();
		user.locked   = view.GetBool("locked");
		user.hardware = HardwareInfo::FromJsonView(view.GetObject("hardware"));
		user.discord  = view.GetString("discord").c_str();
		user.expiry   = (float)view.GetDouble("expiry");
		user.level    = (float)view.GetDouble("level");
		return user;
	}
};