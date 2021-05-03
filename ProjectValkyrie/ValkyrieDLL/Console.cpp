#include "Console.h"
#include "Script.h"

void ConsoleStringLine::ImDraw()
{
	ImGui::TextColored(color, text.c_str());
}

void Console::ImDraw(const PyExecutionContext & ctx)
{
	static object mainModule = import("__main__");
	static object mainNamespace = mainModule.attr("__dict__");

	/// Draw console logs
	auto& io = ImGui::GetIO();
	ImVec2 size = io.DisplaySize;
	size.y = 0.3*size.y;

	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("ConsoleWindow", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	for (auto l : buffer) {
		l->ImDraw();
	}

	ImGui::End();

	/// Draw command line
	ImGui::SetNextWindowPos(ImVec2(0.f, size.y));
	ImGui::Begin("ConsoleCommandLine", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
	
	ImGui::SetNextItemWidth(size.x);
	if (ImGui::InputText("###CommandLine", &line[0], SizeLine, ImGuiInputTextFlags_EnterReturnsTrue)) {

		try {
			
			exec(Strings::Format(
				"from io import StringIO\n"
				"from contextlib import redirect_stdout, redirect_stderr\n"
				"from code import interact\n"
				"f = StringIO()\n"
				"with redirect_stdout(f), redirect_stderr(f):\n"
				"	interact('%s')\n"
				"_result = f.getvalue()", line).c_str(),
				mainNamespace);

			object result = mainNamespace["_result"];
			ConsoleStringLine* strLine = new ConsoleStringLine();
			strLine->text = extract<std::string>(str(result));
			strLine->color = Color::WHITE;

			buffer.push_back(std::shared_ptr<ConsoleLine>(strLine));
		}
		catch (error_already_set) {
			ConsoleStringLine* strLine = new ConsoleStringLine();
			strLine->text = Script::GetPyError();
			strLine->color = Color::RED;

			buffer.push_back(std::shared_ptr<ConsoleLine>(strLine));
		}

		memset(line, 0, SizeLine);
	}

	ImGui::End();
}
