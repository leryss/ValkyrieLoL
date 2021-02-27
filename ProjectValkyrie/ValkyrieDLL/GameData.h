#pragma once
#include <map>
#include <string>
#include <d3d9.h>
#include "UnitInfo.h"
#include "SpellInfo.h"
#include "ItemInfo.h"
#include "SkinInfo.h"

#include "AsyncTask.h"

class LoadDataProgress {

public:
	float       essentialsPercent = 0.0f;
	float       imagesLoadPercent = 0.0f;

	bool        essentialsLoaded  = false;
	bool        allLoaded         = false;
};

class GameDataEssentialsLoad : public AsyncTask {

public:
	void Perform();
};

class GameDataImagesLoad : public AsyncTask {

public:
	void Perform();
};

class GameData {

public:
	/// Gets static info about a game unit, returns nullptr if no info is found
	static UnitInfo*               GetUnit(std::string& str);

	/// Gets spell info about a spell, returns nullptr if no info is found
	static SpellInfo*              GetSpell(std::string& str);

	/// Gets item info about a item given by id, returns nullptr if no item info is found
	static ItemInfo*               GetItem(int id);

	/// Gets a image, returns nullptr if not found
	static PDIRECT3DTEXTURE9       GetImage(std::string& str);

	/// Gets a list of skins for a champion, returns an empty vector on failure
	static std::vector<SkinInfo*>& GetSkins(std::string& name);

	static void                    ImGuiDrawLoader();
	static void                    ImGuiDrawObjects();

	static void                    LoadSpells(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadItems(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadUnits(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadSkins(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadImagesFromZip(const char* zipPath, float& percentValue, float percentEnd);

public:
	static std::shared_ptr<LoadDataProgress>     LoadProgress;

private:
	static const char*                                    FolderData;
	static std::map<std::string, UnitInfo*>               Units;
	static std::map<std::string, std::vector<SkinInfo*>>  Skins;
	static std::map<std::string, SpellInfo*>              Spells;
	static std::map<int, ItemInfo*>                       Items;
	static std::map<std::string, PDIRECT3DTEXTURE9>       Images;
};

