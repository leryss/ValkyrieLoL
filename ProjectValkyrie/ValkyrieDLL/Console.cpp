#include "Console.h"

void ConsoleStringLine::ImDraw()
{
	ImGui::TextColored(color, text.c_str());
}

void Console::ImDraw(const PyExecutionContext & ctx)
{
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

		std::shared_ptr<ConsoleLine> l;

		ConsoleStringLine* strLine = new ConsoleStringLine();
		strLine->text = line;
		strLine->color = Color::WHITE;

		buffer.push_back(std::shared_ptr<ConsoleLine>(strLine));

		memset(line, 0, SizeLine);
	}

	ImGui::End();
}
