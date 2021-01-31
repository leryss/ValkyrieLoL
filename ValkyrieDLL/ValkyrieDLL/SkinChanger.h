#pragma once

#include "Offsets.h"
#include <map>
#include <vector>
#include <set>

class SkinsInfo {

public:
	std::vector<int>         ids;
	std::vector<std::string> names;
	int                      currentSkin = 0; // This is a index for the vectors above not skin id
};

class SkinChanger {

public:

	static void                                  ImGuiDraw();
				                                 
private:		                                 
	static void ReadChampEntries(int champEntryBase, std::set<std::string>& targetChamps);
	static void                                  LoadSkinInfos();
	static bool                                  LoadedSkinInfos;
	static std::map<std::string, SkinsInfo> Skins;
};