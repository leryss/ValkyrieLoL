#include "Console.h"
#include "Script.h"

void ConsoleStringLine::ImDraw()
{
	ImGui::Text(">>>");
	ImGui::SameLine();
	ImGui::TextColored(color, text.c_str());
}

void Console::ImDraw(const PyExecutionContext & ctx, const ScriptManager& smanager)
{
	auto& io = ImGui::GetIO();
	ImVec2 size;
	size.x = io.DisplaySize.x * 0.3f;
	size.y = 50.f;

	/// Draw options: clear & context selection
	ImGui::BeginChild("ConsoleOptions", ImVec2(size), true);

	if (ImGui::Button("Clear")) {
		buffer.clear();
	}

	ImGui::SameLine();
	if (ImGui::BeginCombo("Script Context", smanager.allScripts[selectedContext]->info->id.c_str())) {

		for (int i = 0; i < smanager.allScripts.size(); ++i) {
			auto script = smanager.allScripts[i];
			if (ImGui::Selectable(script->info->id.c_str())) {
				selectedContext = i;
			}
		}
		
		ImGui::EndCombo();
	}
	auto script = smanager.allScripts[selectedContext];
	ImGui::EndChild();

	/// Draw console log
	size.y = io.DisplaySize.y - 200.f;
	ImGui::BeginChild("ConsoleLog", size, true);

	for (auto l : buffer) {
		l->ImDraw();
	}

	ImGui::EndChild();

	/// Draw & process command line
	ImGui::BeginChild("ConsoleCmd", ImVec2(size.x, 50.f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetNextItemWidth(size.x);
	if (ImGui::InputText("###CommandLine", &line[0], SizeLine, ImGuiInputTextFlags_EnterReturnsTrue)) {

		ConsoleStringLine* command = new ConsoleStringLine();
		command->color = Color::CYAN;
		command->text = line;

		buffer.push_back(std::shared_ptr<ConsoleLine>(command));

		try {
			
			script->context["ctx"] = object(boost::ref(ctx));
			exec(Strings::Format(
				"_result = %s", line).c_str(),
				script->context);

			object result = (script->context["_result"]);

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
	ImGui::EndChild();
}

void Console::AddLine(std::shared_ptr<ConsoleLine> line)
{
	buffer.push_back(line);
}

void ConsolePythonObjectLine::ImDraw()
{
	ImGui::Text("<");
	ImGui::SameLine();
	ImDrawObject(obj, (int)&obj);
}

object ConsolePythonObjectLine::ImDrawObject(object & obj, int id)
{
	auto nativePtr = obj.ptr();

	ImGui::PushID(id);

	if (PyList_Check(nativePtr)) {
		list l = extract<list>(obj);
		size_t lsize = len(l);

		if (ImGui::TreeNode(nativePtr, "list (%d)", lsize)) {
			
			for (size_t i = 0; i < lsize; ++i) {
				ImGui::TextColored(Color::YELLOW, "%d", i);
				ImGui::SameLine(200.f);

				object o = l[i];
				l[i] = ImDrawObject(o, id + i*100);
			}
			ImGui::TreePop();
		}
	}

	else if (PyDict_Check(nativePtr)) {
		dict d = extract<dict>(obj);
		size_t dsize = len(d);

		if (ImGui::TreeNode(nativePtr, "dict (%d)", dsize)) {
			list l = d.items();
			size_t lsize = len(l);

			for (size_t i = 0; i < lsize; ++i) {
				tuple t = tuple(l[i]);
				object key = t[0];
				object val = t[1];

				std::string keyStr = extract<std::string>(str(key));
				
				ImGui::TextColored(Color::YELLOW, "%s", keyStr.c_str());
				ImGui::SameLine(200.f);

				d[key] = ImDrawObject(val, id + i*100);
			}

			ImGui::TreePop();
		}
	}
	else if (PyObject_HasAttrString(nativePtr, "__dict__")) {
		object d = obj.attr("__dict__");
		std::string className = extract<std::string>(str(obj.attr("__class__").attr("__name__")));
		ImGui::TextColored(Color::ORANGE, className.c_str());
		ImGui::SameLine();
		ImDrawObject(d, id + 1);
	}
	else if (PyFloat_Check(nativePtr)) {
		float val = extract<float>(obj);
		ImGui::SetNextItemWidth(150.f);
		ImGui::DragFloat("", &val);

		ImGui::PopID();
		return object(val);
	}
	else if (PyLong_Check(nativePtr)) {
		int val = extract<int>(obj);
		ImGui::SetNextItemWidth(150.f);
		ImGui::DragInt("", &val);

		ImGui::PopID();
		return object(val);
	}
	else {
		std::string objAsStr = extract<std::string>(str(obj));
		ImGui::Text(objAsStr.c_str());
	}

	ImGui::PopID();
	return obj;
}

void ConsoleSeparatorLine::ImDraw()
{
	ImGui::Separator();
}
