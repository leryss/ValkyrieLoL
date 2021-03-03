#pragma once

#include "Script.h"
#include "imgui/imgui.h"
#include "PyExecutionContext.h"
#include "ScriptRepository.h"
#include <deque>

class ScriptManager {

public:

	void LoadAllScripts();
	void ExecuteScripts(PyExecutionContext& ctx);
	void ImGuiDrawMenu(PyExecutionContext& ctx);

private:
	void LoadScript(std::shared_ptr<ScriptInfo>& info, std::deque<std::shared_ptr<Script>>& scriptList);
	void ExecuteScripts(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);
	void DrawScriptsMenus(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);

public:
	ScriptRepository                    repository;

	std::deque<std::shared_ptr<Script>> communityScripts;
	std::deque<std::shared_ptr<Script>> coreScripts;
};