#pragma once

#include "Script.h"
#include "imgui/imgui.h"
#include "PyExecutionContext.h"
#include "ScriptRepository.h"
#include <deque>

/// Manages script loading and execution
class ScriptManager {

public:

	/// Loads all scripts from valkyrie scripts folder. Game state is necessary for conditional loading (ex: load only if champion is Alistar)
	void LoadAllScripts(const GameState* gameState);

	/// Executes the main loop of all the scripts. It executes the on load function if the script is just loaded.
	void ExecuteScripts(PyExecutionContext& ctx);

	/// Executes the menu functions of all the scripts.
	void ImGuiDrawMenu(PyExecutionContext& ctx);

private:
	void LoadScript(std::shared_ptr<ScriptInfo>& info, std::deque<std::shared_ptr<Script>>& scriptList);
	void ExecuteScripts(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);
	void DrawScriptsMenus(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);

	/// Draws a footer for the script menu with reload/reset buttons
	void ScriptMenuFooter(std::shared_ptr<Script>& script);

public:
	ScriptRepository                    repository;

	std::deque<std::shared_ptr<Script>> communityScripts;
	std::deque<std::shared_ptr<Script>> coreScripts;
};