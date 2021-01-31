#include "SkinChanger.h"
#include "Valkyrie.h"
#include "Memory.h"

bool                                          SkinChanger::LoadedSkinInfos = false;
std::map<std::string, SkinsInfo>              SkinChanger::Skins;

void SkinChanger::ImGuiDraw()
{
	if (!LoadedSkinInfos)
		LoadSkinInfos();

	
	static auto Update = reinterpret_cast<void(__thiscall*)(void*, bool)>((int)GetModuleHandle(NULL) + Offsets::FnCharacterDataStackUpdate);

	bool changed = false;
	SkinsInfo& info = Skins[Valkyrie::CurrentGameState->player->name];
	
	if (ImGui::Button("Prev")) {
		info.currentSkin = (info.currentSkin - 1 < 0 ? 0 : info.currentSkin - 1);
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		info.currentSkin = (info.currentSkin + 1 >= info.ids.size() ? info.ids.size() - 1 : info.currentSkin + 1);
		changed = true;
	}

	if (ImGui::BeginCombo("Skins", info.names[info.currentSkin].c_str())) {
		bool selected = false;
		for (int i = 0; i < info.names.size(); ++i) {
			if (ImGui::Selectable(info.names[i].c_str(), &selected)) {
				info.currentSkin = i;
				changed = true;
				break;
			}
		}
		ImGui::EndCombo();
	}

	if (changed) {
		int baseAddr = (int)GetModuleHandle(NULL);
		int charDataStack = Valkyrie::CurrentGameState->player->address + Offsets::CharacterDataStack;
		int* charSkinId = (int*)(charDataStack + Offsets::CharacterDataStackSkinId);

		*charSkinId = info.ids[info.currentSkin];
		Update((void*)charDataStack, true);
	}

	ImGui::EndMenu();
	
}

/// Needed because of __try __except 
void ReadString(int nameAddr, SkinsInfo& info) {
	info.names.push_back(Memory::ReadString(nameAddr));
}

void ReadString(int nameAddr, std::string& readInto) {
	readInto = Memory::ReadString(nameAddr);
	readInto = Strings::ToLower(readInto);
}

void ReadSkinEntries(int skinEntryBase, SkinsInfo& info) {
	__try {
		do {
			int nameAddr = ReadInt(skinEntryBase + Offsets::SkinEntryName);
			int skinId = ReadInt(skinEntryBase + Offsets::SkinEntryId);

			info.ids.push_back(skinId);
			ReadString(nameAddr, info);
			Logger::LogAll("Read skin %s", info.names.back().c_str());
			skinEntryBase += 0x10;
		} while (true);
	}
	__except(1) {}
}

void SkinChanger::ReadChampEntries(int champEntryBase, std::set<std::string>& targetChamps) {
	
	static std::string champName;

	__try {
		do {
			int champEntry = ReadInt(champEntryBase);
			ReadString(ReadInt(champEntry + Offsets::ChampionEntryName), champName);

			if (targetChamps.find(champName) != targetChamps.end()) {
				Logger::LogAll("Reading skin names for %s", champName.c_str());
				ReadSkinEntries(ReadInt(champEntry + Offsets::ChampionEntrySkinEntries), Skins[champName]);
			}

			champEntryBase += sizeof(int);
		} while (true);
	}
	__except(1) {}
}

/// Reads the skins for each champion in the game
void SkinChanger::LoadSkinInfos()
{
	Logger::LogAll("Loading skins info");

	std::set<std::string> targetChamps;
	for (auto& champ : Valkyrie::CurrentGameState->champions) {
		targetChamps.insert(champ->name);
		Skins[champ->name] = SkinsInfo();
	}

	int base = (int)GetModuleHandle(NULL);
	int champManager = ReadInt(base + Offsets::ChampionManager);
	int champEntries = ReadInt(champManager + Offsets::ChampionManagerChampionEntries);

	ReadChampEntries(champEntries, targetChamps);

	LoadedSkinInfos = true;
}
