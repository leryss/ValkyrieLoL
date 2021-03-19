#include "InviteInfo.h"

std::shared_ptr<InviteInfo> InviteInfo::FromJsonView(const JsonView & view)
{
	InviteInfo* i = new InviteInfo();

	i->code = view.GetString("code").c_str();
	i->mode = (InviteMode)view.GetInteger("mode");
	i->days = view.GetDouble("days");
	i->level = (UserLevel)view.GetInteger("level");

	return std::shared_ptr<InviteInfo>(i);
}

JsonValue InviteInfo::ToJsonValue() const
{
	JsonValue val;
	val.WithString("code", code.c_str());
	val.WithInteger("mode", mode);
	val.WithDouble("days", days);
	val.WithInteger("level", level);
	return val;
}
