#include "ObjectExplorer.h"
#include "Strings.h"
#include <string>
#include <sstream>
#include <iomanip>

void DrawMatrix(float* matrix, int rows, int cols) {

	ImGui::Columns(cols, NULL);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			ImGui::Button(Strings::Format("%.2f", matrix[i*cols + j]).c_str());
			ImGui::NextColumn();
		}
	}
	ImGui::Columns(1);
}

void DrawGameObject(GameObject& obj) {
	
}

void ObjectExplorer::ImGuiDraw(GameState & state)
{
	ImGui::Begin("Object Explorer");

	ImGui::DragFloat("Game Time", &state.time);

	auto& renderer = state.renderer;
	if (ImGui::TreeNode("Renderer")) {
		
		ImGui::DragInt("Width", &renderer.width);
		ImGui::DragInt("Height", &renderer.height);

		ImGui::Text("View Matrix");
		DrawMatrix(renderer.viewMatrix, 4, 4);
		
		ImGui::Text("Projection Matrix");
		DrawMatrix(renderer.projMatrix, 4, 4);
		
		ImGui::TreePop();
	}

	auto& hud = state.hud;
	if (ImGui::TreeNode("HUD")) {
		
		hud.minimapPosition.ImGuiDraw("Minimap Position");
		hud.minimapSize.ImGuiDraw("Minimap Size");

		ImGui::TreePop();
	}

	auto& objCache = state.objectCache;
	if (ImGui::TreeNode("ObjCache")) {
		
		for (auto& pair : objCache) {
			auto obj = pair.second;
			if (ImGui::TreeNode(&pair.first, "%s (%#010x)", obj->name.c_str(), obj->networkId)) {
				
				obj->ImGuiDraw();
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	ImGui::End();
}
