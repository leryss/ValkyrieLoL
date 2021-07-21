#pragma once

#include "Script.h"
#include "imgui/imgui.h"
#include "PyExecutionContext.h"
#include "ScriptRepository.h"
#include "TextEditor.h"
#include <deque>

/// Manages script loading and execution
class ScriptManager {

public:

	ScriptManager();

	/// Loads all scripts from valkyrie scripts folder. Game state is necessary for conditional loading (ex: load only if champion is Alistar)
	void LoadAllScripts(const GameState* gameState);

	/// Executes the main loop of all the scripts. It executes the on load function if the script is just loaded.
	void ExecuteScripts(PyExecutionContext& ctx);

	/// Executes the menu functions of all the scripts.
	void ImGuiDrawMenu(PyExecutionContext& ctx);

	void ImGuiDrawEditor();

private:
	void        LoadScript(std::shared_ptr<ScriptInfo>& info);
	void        ExecuteScripts(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);
	void        DrawScriptsMenus(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList);
	void        DrawScriptPrefix(std::shared_ptr<Script>& script);
	std::string ReadScript(std::shared_ptr<Script>& script);
	void        SaveScript(std::shared_ptr<Script>& script, const std::string& code);

	/// Draws a footer for the script menu with reload/reset buttons
	void ScriptMenuFooter(std::shared_ptr<Script>& script);

public:
	
	/// Text editor stuff
	TextEditor                          editor;
	size_t                              selectedScriptForEditing = -1;

	ScriptRepository                    repository;

	std::deque<std::shared_ptr<Script>> communityScripts;
	std::deque<std::shared_ptr<Script>> coreScripts;
	std::deque<std::shared_ptr<Script>> allScripts;
};