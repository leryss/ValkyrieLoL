#include "ObjectExplorer.h"
#include "Strings.h"
#include "GameData.h"
#include "Color.h"

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

void DrawGameObject(GameObject* obj) {

	if (ImGui::TreeNode(&obj->networkId, "%s (%#010x)", obj->name.c_str(), obj->networkId)) {

		obj->ImGuiDraw();
		ImGui::TreePop();
	}
}

void ObjectExplorer::ImGuiDraw(GameState & state)
{
	ImGui::Begin("Object Explorer");

	ImGui::DragFloat("Game Time", &state.time);
	if (state.player != nullptr) {
		if (ImGui::TreeNode("Player")) {
			state.player->ImGuiDraw();
			ImGui::TreePop();
		}
	}
	else
		ImGui::TextColored(Color::RED, "No local player");

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
		
		ImGui::Checkbox("Is chat open", &hud.isChatOpen);
		hud.minimapPosition.ImGuiDraw("Minimap Position");
		hud.minimapSize.ImGuiDraw("Minimap Size");

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Champions")) {
		for (auto& obj : state.champions) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Minions")) {
		for (auto& obj : state.minions) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Jungle")) {
		for (auto& obj : state.jungle) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Turrets")) {
		for (auto& obj : state.turrets) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Missiles")) {
		for (auto& obj : state.missiles) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Others")) {
		for (auto& obj : state.others) DrawGameObject(obj.get());
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Static")) {
		GameData::ImGuiDrawObjects();
	}
	
	ImGui::Separator();
	if (state.hovered != nullptr) {
		state.hovered->ImGuiDraw();
		ImGui::TreePop();
	}
	else
		ImGui::TextColored(Color::RED, "Nothing hovered");

	ImGui::End();
}
