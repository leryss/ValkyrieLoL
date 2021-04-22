#pragma once
#include <map>
#include <string>
#include <d3d9.h>
#include "UnitInfo.h"
#include "SpellInfo.h"
#include "ItemInfo.h"
#include "SkinInfo.h"

#include "AsyncTask.h"
#include "AsyncTaskPool.h"

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

	static void                    LoadEverything();
	static void                    LoadEssentials();

	/// Gets static info about a game unit, returns nullptr if no info is found
	static UnitInfo*               GetUnit(std::string& str);

	/// Gets spell info about a spell, returns nullptr if no info is found
	static SpellInfo*              GetSpell(std::string& str);

	/// Gets item info about a item given by id, returns nullptr if no item info is found
	static ItemInfo*               GetItem(int id);

	/// Gets a image, returns nullptr if not found
	static PDIRECT3DTEXTURE9       GetImage(std::string& str);
	static PDIRECT3DTEXTURE9       GetImage(const char* str);

	/// Gets a list of skins for a champion, returns an empty vector on failure
	static std::vector<SkinInfo*>& GetSkins(std::string& name);

	/// Checks if there is a wall at the specified position
	static bool                    IsWallAt(const Vector3& worldPos);

	static void                    ImGuiDrawObjects();

	static void                    LoadWallMask(const char* filename, float& percentValue, float percentEnd);
	static void                    LoadSpells(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadItems(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadUnits(const char* fileName, float& percentValue, float percentEnd);
	static void                    LoadImagesFromZip(const char* zipPath, float& percentValue, float percentEnd);

public:
	static bool                    EssentialsLoaded;
	static bool                    EverythingLoaded;

private:
	static bool*                                          WallMask;
	static const char*                                    FolderData;
	static std::map<std::string, UnitInfo*>               Units;
	static std::map<std::string, std::vector<SkinInfo*>>  Skins;
	static std::map<std::string, SpellInfo*>              Spells;
	static std::map<int, ItemInfo*>                       Items;
	static std::map<std::string, PDIRECT3DTEXTURE9>       Images;

	static AsyncTaskPool*                                 TaskPool;
};

