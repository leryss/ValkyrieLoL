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
	info->averageRating = json.ValueExists("average_rating") ? json.GetDouble("average_rating") : 0.f;
	info->numRatings    = json.ValueExists("num_ratings") ? json.GetInteger("num_ratings") : 0;
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
	val.WithDouble("average_rating", averageRating);
	val.WithInteger("num_ratings", numRatings);
	return val;
}