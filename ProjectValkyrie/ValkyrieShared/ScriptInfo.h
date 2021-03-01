#pragma once
#include <string>
#include <aws/core/utils/json/JsonSerializer.h>

using namespace Aws::Utils::Json;

class ScriptInfo {
	
public:
	std::string id;
	std::string name;
	std::string description;
	std::string author;
	std::string champion;
	int         downloads;
	float       lastUpdate;

	static ScriptInfo   FromJsonView(const JsonView& json);
	JsonValue           ToJsonValue() const;
};