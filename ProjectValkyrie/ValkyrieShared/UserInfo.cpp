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
	user.resetHardware = view.GetBool("reset_hardware");
	return user;
}

JsonValue UserInfo::ToJsonValue() const
{
	auto json = JsonValue();

	json.WithString("name",     name.c_str());
	json.WithBool("locked",     locked);
	json.WithObject("hardware", hardware.ToJsonValue());
	json.WithString("discord",  discord.c_str());
	json.WithDouble("expiry",   expiry);
	json.WithDouble("level",    level);
	json.WithBool("reset_hardware", resetHardware);

	return json;
}
