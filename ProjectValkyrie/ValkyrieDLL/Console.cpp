#include "Console.h"
#include "Script.h"

void ConsoleStringLine::ImDraw()
{
	ImGui::Text(">>>");
	ImGui::SameLine();
	ImGui::TextColored(color, text.c_str());
}

void Console::ImDraw(const PyExecutionContext & ctx)
{
	static object mainModule = import("__main__");
	static object mainNamespace = mainModule.attr("__dict__");

	/// Draw console logs
	auto& io = ImGui::GetIO();
	ImVec2 size = io.DisplaySize;
	size.y = size.y - 100;
	size.x *= 0.3f;

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

		ConsoleStringLine* command = new ConsoleStringLine();
		command->color = Color::CYAN;
		command->text = line;

		buffer.push_back(std::shared_ptr<ConsoleLine>(command));

		try {
			
			mainNamespace["ctx"] = object(boost::ref(ctx));
			exec(Strings::Format(
				"_result = %s", line).c_str(),
				mainNamespace);

			object result = mainNamespace["_result"];

			ConsolePythonObjectLine* commandResp = new ConsolePythonObjectLine();
			commandResp->obj = result;

			buffer.push_back(std::shared_ptr<ConsoleLine>(commandResp));
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

void ConsolePythonObjectLine::ImDraw()
{
	ImGui::Text("<");
	ImGui::SameLine();

	if (PyList_Check(obj.ptr())) {
		const char* objAsStr = extract<const char*>(str(obj));

		if (ImGui::TreeNode(objAsStr)) {
			list l = list(obj);
			for (size_t i = 0; i < len(l); ++i) {
				object o = l[i];
				ImDrawObject(o);
			}
			ImGui::TreePop();
		}
	}
	else {
		ImDrawObject(obj);
	}
}

void ConsolePythonObjectLine::ImDrawObject(object & obj)
{
	std::string objAsStr = extract<std::string>(str(obj));

	if(PyObject_HasAttrString(obj.ptr(), "menu_draw")) {
		if (ImGui::TreeNode(objAsStr.c_str())) {
			obj.attr("menu_draw")();
			ImGui::TreePop();
		}
	} else
		ImGui::TextColored(Color::WHITE, objAsStr.c_str());
}

void ConsoleSeparatorLine::ImDraw()
{
	ImGui::Separator();
}
