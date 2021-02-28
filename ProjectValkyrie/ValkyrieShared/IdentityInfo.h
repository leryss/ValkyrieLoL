#pragma once
#include "HardwareInfo.h"

class IdentityInfo {

public:
	IdentityInfo() {}
	IdentityInfo(std::string name, std::string pass, HardwareInfo hardware) {
		this->name = name;
		this->pass = pass;
		this->hardware = hardware;
	}

	std::string name;
	std::string pass;
	HardwareInfo hardware;
};