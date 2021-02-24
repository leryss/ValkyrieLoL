#pragma once

#include "Script.h"
#include "imgui/imgui.h"
#include "PyExecutionContext.h"
#include <deque>

class ScriptManager {

public:

	void LoadScriptsFromFolder(std::string& folderPath);
	void ExecuteScripts(PyExecutionContext& ctx);
	void ImGuiDrawMenu(PyExecutionContext& ctx);

public:
	std::deque<std::shared_ptr<Script>> scripts;
};