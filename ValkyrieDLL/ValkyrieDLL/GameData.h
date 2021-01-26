#pragma once
#include <map>
#include <string>
#include <d3d9.h>
#include "UnitInfo.h"
#include "SpellInfo.h"
#include "ItemInfo.h"

class LoadDataProgress {

public:
	float       percentDone      = 0.0f;
	const char* currentlyLoading = "Loading not started";
	bool        complete         = false;
};

class GameData {

public:
	static void LoadAsync();
	
private:
	static void Load();
	static void LoadSpells(const char* fileName, float percentEnd);
	static void LoadItems(const char* fileName, float percentEnd);
	static void LoadUnits(const char* fileName, float percentEnd);
	static void LoadImages(const char* folderName, float percentEnd);

	static const char* FolderData;
public:
	static std::shared_ptr<LoadDataProgress>     LoadProgress;

	static std::map<std::string, UnitInfo*>          Units;
	static std::map<std::string, SpellInfo*>         Spells;
	static std::map<int, ItemInfo*>                  Items;
	static std::map<std::string, PDIRECT3DTEXTURE9*> Images;
};