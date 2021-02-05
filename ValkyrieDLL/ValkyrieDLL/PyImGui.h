#pragma once
#include "HKey.h"
#include "imgui/imgui.h"
#include "Color.h"

#include <boost/python.hpp>
#include "InputController.h"

using namespace boost::python;

/// Interface used by python scripts for creating UIs with imgui.
class PyImGui {

public:

	void Begin(const char* name) {
		ImGui::Begin(name);
	}

	void End() {
		ImGui::End();
	}

	bool Button(const char* text) {
		return ImGui::Button(text);
	}

	bool ColorButton(const char* text, object color) {
		return ImGui::ColorButton(text, extract<ImVec4>(color));
	}

	bool Checkbox(const char* text, bool enabled) {
		ImGui::Checkbox(text, &enabled);
		return enabled;
	}

	void Text(const char* text) {
		ImGui::Text(text);
	}

	void TextColored(const char* text, object color) {
		ImGui::TextColored(extract<ImVec4>(color), text);
	}

	void LabelText(const char* label, const char* text) {
		ImGui::LabelText(label, text);
	}

	void LabelTextColored(const char* label, const char* text, object color) {
		ImVec4 col = extract<ImVec4>(color);

		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::LabelText(label, text);
		ImGui::PopStyleColor();
	}

	ImVec4 ColorPicker(const char* label, object color) {
		ImVec4 col = extract<ImVec4>(color);
		ImGui::ColorPicker4(label, (float*)&col);
		return col;
	}

	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ImageOverloads, Image, 2, 3);
	void Image(const char* image, const Vector2& size, ImVec4 color = Color::WHITE) {
		std::string imgStr(image);
		ImGui::Image(GameData::GetImage(imgStr), (ImVec2&)size, ImVec2(0, 0), ImVec2(1, 1), color);
	}

	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DragIntOverloads, DragInt, 2, 5);
	int DragInt(const char* text, int i, int step = 1, int minVal = 0, int maxVal = 0) {
		ImGui::DragInt(text, &i, (float)step, minVal, maxVal);
		return i;
	}

	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DragFloatOverloads, DragFloat, 2, 5);
	float DragFloat(const char* text, float i, float step = 1, float minVal = 0, float maxVal = 0) {
		ImGui::DragFloat(text, &i, step, minVal, maxVal);
		return i;
	}

	float SliderFloat(const char* label, float val, float valMin, float valMax) {
		ImGui::SliderFloat(label, &val, valMin, valMax);
		return val;
	}

	void Separator() {
		ImGui::Separator();
	}

	bool BeginMenu(const char* label) {
		return ImGui::BeginMenu(label);
	}

	void EndMenu() {
		ImGui::EndMenu();
	}

	bool CollapsingHeader(const char* text) {
		return ImGui::CollapsingHeader(text);
	}

	bool TreeNode(const char* text) {
		return ImGui::TreeNode(text);
	}

	void SetNextItemOpen() {
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	}

	void TreePop() {
		ImGui::TreePop();
	}

	void SameLine() {
		ImGui::SameLine();
	}

	void BeginGroup() {
		ImGui::BeginGroup();
	}

	void EndGroup() {
		ImGui::EndGroup();
	}

	int ListBox(const char* label, list items, int chosen) {
		static std::vector<const char*> buffer;

		buffer.clear();
		int size = len(items);
		for (int i = 0; i < size; ++i)
			buffer.push_back(extract<const char*>(str(items[i])));

		ImGui::ListBox(label, &chosen, buffer.data(), size, size);

		return chosen;
	}

	int Combo(const char* label, list items, int selected) {
		static std::vector<const char*> buffer;

		buffer.clear();
		int size = len(items);
		for (int i = 0; i < size; ++i)
			buffer.push_back(extract<const char*>(str(items[i])));

		if (ImGui::BeginCombo(label, buffer[selected], ImGuiComboFlags_HeightLargest)) {
			bool bsel = false;
			for (int i = 0; i < size; ++i) {
				if (ImGui::Selectable(buffer[i], &bsel))
					selected = i;
			}
			ImGui::EndCombo();
		}
		return selected;
	}

	int KeySelect(const char* label, int key) {
		return InputController::ImGuiKeySelect(label, key);
	}
};