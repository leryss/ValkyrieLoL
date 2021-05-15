#pragma once
#include <string>
#include <aws/core/utils/json/JsonSerializer.h>

using namespace Aws::Utils::Json;

class SessionInfo {

public:
	std::string summonerName;
	float timestamp;

	static std::shared_ptr<SessionInfo> FromJsonView(const JsonView& view);
	JsonValue ToJsonValue() const;
};