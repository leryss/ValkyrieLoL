#include "SessionInfo.h"

std::shared_ptr<SessionInfo> SessionInfo::FromJsonView(const JsonView & view)
{
	std::shared_ptr<SessionInfo> o = std::shared_ptr<SessionInfo>(new SessionInfo());
	o->timestamp    = (float)view.GetDouble("timestamp");
	o->summonerName = view.GetString("summoner_name").c_str();
}

JsonValue SessionInfo::ToJsonValue() const
{
	auto json = JsonValue();
	json.WithDouble("timestamp", timestamp);
	json.WithString("summoner_name", summonerName.c_str());

	return json;
}
