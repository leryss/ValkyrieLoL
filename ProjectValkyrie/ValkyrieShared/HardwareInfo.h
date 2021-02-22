#pragma once
#include <string>
#include <aws/core/utils/json/JsonSerializer.h>

using namespace Aws::Utils::Json;

class HardwareInfo {

public:
	std::string         cpuInfo;
	std::string         gpuInfo;
	std::string         ramInfo;
	std::string         systemName;

	static HardwareInfo Calculate();
	static HardwareInfo FromJsonView(const JsonView& json);
	JsonValue           ToJsonValue() const;
	std::string         ToJsonString() const;
};