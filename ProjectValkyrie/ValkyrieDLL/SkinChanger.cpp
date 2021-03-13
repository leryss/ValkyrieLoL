#include "SkinChanger.h"
#include "Valkyrie.h"
#include "Memory.h"
#include "Color.h"

int SkinChanger::CurrentSkinIndex   = 0;
int SkinChanger::CurrentChromaIndex = -1;
int SkinChanger::CurrentSkinId      = -1;

void SkinChanger::ImGuiDraw()
{
	std::vector<SkinInfo*>& skins = GameData::GetSkins(Valkyrie::CurrentGameState->player->name);
	if (skins.size() == 0 || skins.size() <= CurrentSkinIndex)
		return;

	bool changed = false;
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
		if (CurrentChromaIndex >= 0)
			CurrentSkinId = skins[CurrentSkinIndex]->chromas[CurrentChromaIndex].id;
		else
			CurrentSkinId = skins[CurrentSkinIndex]->id;
	}
}

void SkinChanger::Refresh()
{
	static auto UpdateSkin = reinterpret_cast<void(__thiscall*)(void*, bool)>((int)GetModuleHandle(NULL) + Offsets::FnCharacterDataStackUpdate);

	/// If champ is dead no need to check or change skin
	if (CurrentSkinId == -1 || Valkyrie::CurrentGameState->player->isDead)
		return;

	/// Check if skin id was changed in memory
	int charDataStack = Valkyrie::CurrentGameState->player->address + Offsets::CharacterDataStack;
	int* charSkinId = (int*)(charDataStack + Offsets::CharacterDataStackSkinId);

	/// Update skin if necessary
	if (*charSkinId != CurrentSkinId) {
		*charSkinId = CurrentSkinId;
		UpdateSkin((void*)charDataStack, true);
		Logger::Info("[skin_changer] Changed skin to %d", CurrentSkinId);
	}
}
