#include "UserInfo.h"

UserInfo UserInfo::FromJsonView(const JsonView & view)
{
	UserInfo user;

	user.name = view.GetString("name").c_str();
	user.locked = view.GetBool("locked");
	user.hardware = HardwareInfo::FromJsonView(view.GetObject("hardware"));
	user.discord = view.GetString("discord").c_str();
	user.expiry = (float)view.GetDouble("expiry");
	user.level = (float)view.GetDouble("level");
	return user;
}
