#include "ScriptManager.h"
#include <algorithm>
#include <windows.h>
#include "Color.h"
#include "Logger.h"
#include "Paths.h"
#include "PyExecutionContext.h"
#include "Debug.h"
#include "ValkyrieShared.h"

ScriptManager::ScriptManager()
{
	editor.SetTabSize(4);
	editor.SetShowWhitespaces(false);

	TextEditor::LanguageDefinition ld;
	ld.mAutoIndentation = true;
	ld.mCaseSensitive   = true;
	ld.mCommentStart    = "'''";
	ld.mCommentEnd      = "'''";
	ld.mSingleLineComment = "#";
	ld.mName            = "Python";
	
	static const char* const keywords[] = {
		"False" ,     "await"   ,   "else"   ,    "import"  ,   "pass"  ,
		"None"  ,     "break"   ,   "except" ,    "in"      ,   "raise" ,
		"True"  ,     "class"   ,   "finally",    "is"      ,   "return",
		"and"   ,     "continue",   "for"    ,    "lambda"  ,   "try"   ,
		"as"    ,     "def"     ,   "from"   ,    "nonlocal",   "while" ,
		"assert",     "del"     ,   "global" ,    "not"     ,   "with"  ,
		"async" ,     "elif"    ,   "if"     ,    "or"      ,   "yield" 
	};

	for (auto& k : keywords)
		ld.mKeywords.insert(k);

	static const char* const identifiers[] = {
		"abs", "delattr", "hash", "memoryview", "set", "all", "dict", "help", "min", "setattr", "any", "dir", "hex", "next", "slice", "ascii", "divmod", "id", "object", "sorted", "bin", "enumerate", "input", "oct", "staticmethod", "bool", "eval", "int", "open", "str", "breakpoint", "exec", "isinstance", "ord", "sum", "bytearray", "filter", "issubclass", "pow", "super", "bytes", "float", "iter", "print", "tuple", "callable", "format", "len", "property", "type", "chr", "frozenset", "list", "range", "vars", "classmethod", "getattr", "locals", "repr", "zip", "compile", "globals", "map", "reversed", "__import__", "complex", "hasattr", "max", "round"
	};

	for (auto& k : identifiers)
	{
		TextEditor::Identifier id;
		id.mDeclaration = "Built-in function";
		ld.mIdentifiers.insert(std::make_pair(std::string(k), id));
	}

	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"",                                              TextEditor::PaletteIndex::String));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("\\\'[^\\\']*\\\'",                                                        TextEditor::PaletteIndex::String));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?",                                        TextEditor::PaletteIndex::Number));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",              TextEditor::PaletteIndex::Number));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?",                                              TextEditor::PaletteIndex::Number));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*",                                                  TextEditor::PaletteIndex::Identifier));
	ld.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", TextEditor::PaletteIndex::Punctuation));

	editor.SetLanguageDefinition(ld);
}

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

	/// Fill allScripts array
	allScripts.clear();
	for (auto script : coreScripts)
		allScripts.push_back(script);
	for (auto script : communityScripts)
		allScripts.push_back(script);
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

void ScriptManager::ImGuiDrawEditor()
{
	if (allScripts.size() == 0)
		return;

	auto displaySize = ImGui::GetIO().DisplaySize;
	float width = 0.4f * displaySize.x;

	ImGui::BeginChild("EditorOptions", ImVec2(width, 100));
	if (ImGui::BeginCombo("Script to Edit", allScripts[selectedScriptForEditing]->info->id.c_str())) {
	
		for (int i = 0; i < allScripts.size(); ++i) {
			auto script = allScripts[i];
			if (ImGui::Selectable(script->info->id.c_str(), i == selectedScriptForEditing)) {
				selectedScriptForEditing = i;

				std::ifstream in(Paths::GetScriptPath(script->info->id));
				std::string code((
					std::istreambuf_iterator<char>(in)),
					std::istreambuf_iterator<char>());
				editor.SetText(code);
				break;
			}
		}

		ImGui::EndCombo();
	}
	ImGui::EndChild();

	ImGui::PushFont(ValkyrieShared::FontConsolas);
	editor.Render("TextEditor", ImVec2(width, 0.6*displaySize.y));
	ImGui::PopFont();
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
		DBG_INFO("ScriptManager: Executing script %s", script->info->id.c_str())
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
		const char* scriptName = script->info->name.c_str();
		ImGui::Text(" ");
		ImGui::SameLine();

		bool errored = !script->error.empty();
		if(errored)
			ImGui::PushStyleColor(ImGuiCol_Text, Color::RED);

		if (ImGui::BeginMenu(scriptName)) {
			if (!errored) {
				ctx.SetScript(script.get());

				DBG_INFO("ScriptManager: Drawing UI for script %s", script->info->id.c_str())
				script->Execute(ctx, ON_MENU);
				if (script->config.IsTimeToSave()) {
					script->Execute(ctx, ON_SAVE);
					script->config.Save();
				}
			}
			else {
				ImGui::Text(script->error.c_str());
			}

			ScriptMenuFooter(script);
			ImGui::EndMenu();
		}

		if (errored)
			ImGui::PopStyleColor();
	}
}

void ScriptManager::ScriptMenuFooter(std::shared_ptr<Script>& script)
{
	if (ImGui::Button("Reload"))
		script->Load(script->info);
	ImGui::SameLine();
	if (ImGui::Button("Reset to Defaults")) {
		script->config.Reset();
		script->config.Save();
		script->Load(script->info);
	}
}
