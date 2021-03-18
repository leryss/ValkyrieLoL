#include "ScriptManager.h"
#include <algorithm>
#include <windows.h>
#include "Color.h"
#include "Logger.h"
#include "Paths.h"
#include "PyExecutionContext.h"

void ScriptManager::LoadAllScripts(const GameState* gameState)
{
	Logger::Info("Loading scripts");
	repository.LoadLocalEntries(Paths::ScriptsIndex);
	
	object sys = import("sys");
	sys.attr("path").attr("insert")(0, Paths::Scripts.c_str());

	for (auto& pair : repository.entries) {
		auto scriptInfo = pair.second->local;
		if (scriptInfo == nullptr)
			continue;
		
		if (scriptInfo->champion != "all" && gameState->player->name != scriptInfo->champion)
			continue;

		if (scriptInfo->author == "TeamValkyrie")
			LoadScript(scriptInfo, coreScripts);
		else
			LoadScript(scriptInfo, communityScripts);
	}

	std::sort(coreScripts.begin(), coreScripts.end(),
		[](const std::shared_ptr<Script>& s1, const std::shared_ptr<Script>& s2) {
			return (s1->info->id.compare(s2->info->id) < 0 ? false : true);
		}
	);

	std::sort(communityScripts.begin(), communityScripts.end(),
		[](const std::shared_ptr<Script>& s1, const std::shared_ptr<Script>& s2) {
			return (s1->info->id.compare(s2->info->id) < 0 ? false : true);
		}
	);
}

void ScriptManager::ExecuteScripts(PyExecutionContext & ctx)
{
	ExecuteScripts(ctx, coreScripts);
	ExecuteScripts(ctx, communityScripts);
}

void ScriptManager::ImGuiDrawMenu(PyExecutionContext & ctx)
{
	ImGui::TextColored(Color::PURPLE, "Official Scripts");
	DrawScriptsMenus(ctx, coreScripts);
	
	ImGui::Separator();
	ImGui::TextColored(Color::PURPLE, "Community Scripts");
	DrawScriptsMenus(ctx, communityScripts);
}

void ScriptManager::LoadScript(std::shared_ptr<ScriptInfo> & info, std::deque<std::shared_ptr<Script>>& scriptList)
{
	auto it = std::find_if(scriptList.begin(), scriptList.end(), [&info](const auto& s) { return s->info->id.compare(info->id) == 0; });
	if (it != scriptList.end())
		(*it)->Load(info);
	else {
		std::shared_ptr<Script> script(new Script());
		script->Load(info);
		scriptList.push_back(script);
	}
}

void ScriptManager::ExecuteScripts(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList)
{
	for (auto script : scriptList) {
		if (script->error.empty()) {
			ctx.SetScript(script.get());
			if (script->neverExecuted)
				script->Execute(ctx, ON_LOAD);
			else
				script->Execute(ctx, ON_LOOP);
		}
	}
}

void ScriptManager::DrawScriptsMenus(PyExecutionContext & ctx, std::deque<std::shared_ptr<Script>>& scriptList)
{
	for (auto script : scriptList) {
		bool errored = !script->error.empty();
		if (errored)
			ImGui::PushStyleColor(ImGuiCol_Text, Color::RED);

		const char* scriptName = script->info->name.c_str();

		ImGui::Text(" ");
		ImGui::SameLine();
		if (ImGui::BeginMenu(scriptName)) {
			if (errored) {
				if (ImGui::Button("Reload"))
					script->Load(script->info);
				ImGui::SameLine();
				if (ImGui::Button("Clear cfg & reload")) {
					script->config.Reset();
					script->config.Save();
					script->Load(script->info);
				}
				ImGui::TextColored(Color::RED, script->error.c_str());
			}
			else {
				ctx.SetScript(script.get());

				script->Execute(ctx, ON_MENU);
				if (script->config.IsTimeToSave()) {
					script->Execute(ctx, ON_SAVE);
					script->config.Save();
				}
			}
			ImGui::EndMenu();
		}

		if (errored)
			ImGui::PopStyleColor();
	}
}
