#include "ScriptManager.h"
#include <algorithm>
#include <windows.h>
#include "Color.h"
#include "Logger.h"
#include "PyExecutionContext.h"

void ScriptManager::LoadScriptsFromFolder(std::string & folderPath)
{
	Logger::LogAll("Loading scripts from %s", folderPath.c_str());

	object sys = import("sys");
	sys.attr("path").attr("insert")(0, folderPath.c_str());

	WIN32_FIND_DATAA findData;
	HANDLE hFind;

	hFind = FindFirstFileA((folderPath + "\\*.py").c_str(), &findData);
	do {
		if (hFind != INVALID_HANDLE_VALUE) {
			
			std::string fileName = findData.cFileName;
			fileName.erase(fileName.find(".py"), 3);

			auto it = std::find_if(scripts.begin(), scripts.end(), [&fileName](const auto& s) { return s->fileName.compare(fileName) == 0; });
			if (it != scripts.end())
				(*it)->LoadFromFile(fileName);
			else {
				std::shared_ptr<Script> script(new Script());
				script->LoadFromFile(fileName);
				scripts.push_back(script);
			}
		}
	} while (FindNextFileA(hFind, &findData));
	
	std::sort(scripts.begin(), scripts.end(),
		[](const std::shared_ptr<Script>& s1, const std::shared_ptr<Script>& s2) {
			return (s1->fileName.compare(s2->fileName) < 0 ? false : true);
		}
	);
}

void ScriptManager::ExecuteScripts(PyExecutionContext & ctx)
{
	for (auto script : scripts) {
		if (script->error.empty()) {
			ctx.SetScript(script.get());
			if (script->neverExecuted)
				script->Execute(ctx, ON_LOAD);
			else
				script->Execute(ctx, ON_LOOP);
		}
	}
}

void ScriptManager::ImGuiDrawMenu(PyExecutionContext & ctx)
{
	for (auto script : scripts) {
		bool errored = !script->error.empty();
		if (errored)
			ImGui::PushStyleColor(ImGuiCol_Text, Color::RED);

		const char* scriptName = (script->prettyName.empty() ? script->fileName.c_str() : script->prettyName.c_str());

		ImGui::Image(GameData::GetImage(script->icon), ImVec2(15, 15));
		ImGui::SameLine();
		if (ImGui::BeginMenu(scriptName)) {
			if (errored)
				ImGui::TextColored(Color::RED, script->error.c_str());
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
