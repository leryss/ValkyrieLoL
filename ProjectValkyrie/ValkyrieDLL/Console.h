#pragma once
#include "Color.h"
#include "PyExecutionContext.h"
#include "ScriptManager.h"
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

class ConsoleSeparatorLine : ConsoleLine {
public:
	virtual void ImDraw() override;
};

class ConsolePythonObjectLine : public ConsoleLine {

public:
	virtual void ImDraw() override;
private:
	object ImDrawObject(object& obj, int id);

public:
	object obj;
};

class Console {

public:
	void ImDraw(const PyExecutionContext& ctx, const ScriptManager& smanager);
	void AddLine(std::shared_ptr<ConsoleLine> line);

private:

	std::vector<std::shared_ptr<ConsoleLine>> buffer;
	const static size_t SizeLine = 1024;
	char line[SizeLine];
	int  selectedContext = 0;
};