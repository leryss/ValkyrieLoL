#include "SkinChanger.h"
#include "Valkyrie.h"
#include "Memory.h"
#include "Color.h"

int SkinChanger::CurrentSkinIndex   = 0;
int SkinChanger::CurrentChromaIndex = -1;

void SkinChanger::ImGuiDraw()
{
	static auto Update = reinterpret_cast<void(__thiscall*)(void*, bool)>((int)GetModuleHandle(NULL) + Offsets::FnCharacterDataStackUpdate);

	bool changed = false;
	
	std::vector<SkinInfo*>& skins = GameData::GetSkins(Valkyrie::CurrentGameState->player->name);
	if (skins.size() == 0 || skins.size() <= CurrentSkinIndex)
		return;

	/*if (ImGui::Button("Prev")) {
		info.currentSkin = (info.currentSkin - 1 < 0 ? 0 : info.currentSkin - 1);
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		info.currentSkin = (info.currentSkin + 1 >= info.ids.size() ? info.ids.size() - 1 : info.currentSkin + 1);
		changed = true;
	}*/

	if (ImGui::BeginCombo("Skins", skins[CurrentSkinIndex]->name.c_str(), ImGuiComboFlags_HeightLargest)) {
		bool selected = false;
		for (size_t i = 0; i < skins.size(); ++i) {
			if (ImGui::Selectable(skins[i]->name.c_str(), &selected)) {
				CurrentSkinIndex = i;
				CurrentChromaIndex = -1;
				changed = true;
				break;
			}
		}
		ImGui::EndCombo();
	}

	auto& chromas = skins[CurrentSkinIndex]->chromas;
	if (chromas.size() > 0)
		ImGui::Text("Chromas");
	for (size_t i = 0; i < chromas.size(); ++i) {
		ImGui::PushID(chromas[i].id);
		if (ImGui::ColorButton(" ", chromas[i].color)) {
			CurrentChromaIndex = i;
			changed = true;
		}
		ImGui::SameLine();
		ImGui::PopID();
	}

	if (changed) {
		int baseAddr = (int)GetModuleHandle(NULL);
		int charDataStack = Valkyrie::CurrentGameState->player->address + Offsets::CharacterDataStack;
		int* charSkinId = (int*)(charDataStack + Offsets::CharacterDataStackSkinId);

		if (CurrentChromaIndex >= 0)
			*charSkinId = skins[CurrentSkinIndex]->chromas[CurrentChromaIndex].id;
		else
			*charSkinId = skins[CurrentSkinIndex]->id;
		Update((void*)charDataStack, true);
	}

	ImGui::EndMenu();
	
}