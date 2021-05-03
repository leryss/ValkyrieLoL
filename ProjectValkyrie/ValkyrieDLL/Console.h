#pragma once
#include "Color.h"
#include "PyExecutionContext.h"
#include <string>

class ConsoleLine {

public:
	virtual void ImDraw() = 0;
};

class ConsoleStringLine: public ConsoleLine {
	
public:
	virtual void ImDraw() override;

public:
	ImVec4      color;
	std::string text;

};

class Console {

public:
	void ImDraw(const PyExecutionContext& ctx);

private:

	std::vector<std::shared_ptr<ConsoleLine>> buffer;
	const static size_t SizeLine = 1024;
	char line[SizeLine];
};