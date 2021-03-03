#include "ScriptInfo.h"
#include "Paths.h"

std::shared_ptr<ScriptInfo> ScriptInfo::FromJsonView(const JsonView & json)
{
	std::shared_ptr<ScriptInfo> info = std::shared_ptr<ScriptInfo>(new ScriptInfo());

	info->name        = json.GetString("name").c_str();
	info->id          = json.GetString("id").c_str();
	info->author      = json.GetString("author").c_str();
	info->description = json.GetString("description").c_str();
	info->champion    = json.GetString("champion").c_str();
	info->lastUpdate  = json.GetDouble("last_updated");
	return info;
}

JsonValue ScriptInfo::ToJsonValue() const
{
	JsonValue val;

	val.WithString("name", name.c_str());
	val.WithString("id", id.c_str());
	val.WithString("author", author.c_str());
	val.WithString("description", description.c_str());
	val.WithString("champion", champion.c_str());
	val.WithDouble("last_updated", lastUpdate);

	return val;
}