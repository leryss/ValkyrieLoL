#include "GameReader.h"
#include "GameData.h"
#include "Globals.h"
#include "Logger.h"
#include "Strings.h"
#include "Valkyrie.h"
#include "json/json.h"

#include "d3dx9tex.h"
#include <filesystem>
#include <thread>

#include <windows.h>

using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

const char*                       GameData::FolderData = "data";
std::shared_ptr<LoadDataProgress> GameData::LoadProgress(new LoadDataProgress());

std::map<std::string, UnitInfo*>          GameData::Units;
std::map<std::string, SpellInfo*>         GameData::Spells;
std::map<std::string, PDIRECT3DTEXTURE9>  GameData::Images;
std::map<int, ItemInfo*>                  GameData::Items;


void GameData::LoadAsync()
{
	std::thread loadThread(GameData::Load);
	loadThread.detach();
}

UnitInfo * GameData::GetUnit(std::string & str)
{
	auto find = Units.find(str);
	return (find == Units.end() ? nullptr : find->second);
}

SpellInfo * GameData::GetSpell(std::string & str)
{
	auto find = Spells.find(str);
	return (find == Spells.end() ? nullptr : find->second);
}

ItemInfo * GameData::GetItem(int id)
{
	auto find = Items.find(id);
	return (find == Items.end() ? nullptr : find->second);
}

PDIRECT3DTEXTURE9 GameData::GetImage(std::string & str)
{
	auto find = Images.find(str);
	return (find == Images.end() ? nullptr : find->second);
}

void GameData::Load()
{
	Valkyrie::WaitForOverlayToInit();

	LoadProgress->currentlyLoading = "Loading Spell Database";
	Logger::LogAll(LoadProgress->currentlyLoading);
	LoadSpells("SpellData.json", 0.1f);
	LoadSpells("SpellDataCustom.json", 0.15f);
	Logger::LogAll("Loaded %zu spells", Spells.size());

	LoadProgress->currentlyLoading = "Loading Unit Database";
	Logger::LogAll(LoadProgress->currentlyLoading);
	LoadUnits("UnitData.json", 0.3f);
	Logger::LogAll("Loaded %zu units", Units.size());

	LoadProgress->currentlyLoading = "Loading Item Database";
	Logger::LogAll(LoadProgress->currentlyLoading);
	LoadItems("ItemData.json", 0.4f);
	Logger::LogAll("Loaded %zu items", Items.size());

	LoadProgress->currentlyLoading = "Loading Image Database";
	Logger::LogAll(LoadProgress->currentlyLoading);
	LoadImages("icons_spells", 0.8f);
	LoadImages("icons_champs", 0.95f);
	LoadImages("icons_extra", 1.f);
	Logger::LogAll("Loaded %zu images", Images.size());

	Logger::LogAll("Static data loading complete");
	LoadProgress->percentDone = 1.f;
	LoadProgress->complete = true;
}

void GameData::LoadSpells(const char* fileName, float percentEnd)
{
	fs::path path = Globals::WorkingDir;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::LogAll("Couldn't open file %s", path.generic_string().c_str());
		return;
	}

	json j;
	file >> j;

	float step = (percentEnd - LoadProgress->percentDone) / j.size();
	for (auto spell : j) {
		SpellInfo* info = new SpellInfo();

		std::string strIcon = spell["icon"].get<std::string>();
		std::string strName = spell["name"].get<std::string>();

		info->flags        = (SpellFlags)spell["flags"].get<int>();
		info->delay        = spell["delay"].get<float>();
		info->height       = spell["height"].get<float>();
		info->icon         = Strings::ToLower(strIcon);
		info->name         = Strings::ToLower(strName);
		info->width        = spell["width"].get<float>();
		info->castRange    = spell["castRange"].get<float>();
		info->castRadius   = spell["castRadius"].get<float>();
		info->speed        = spell["speed"].get<float>();
		info->travelTime   = spell["travelTime"].get<float>();
		info->flags        = (SpellFlags)(info->flags | (spell["projectDestination"] ? ProjectedDestination : 0));

		LoadProgress->percentDone += step;
		Spells[info->name] = info;
	}
}

void GameData::LoadItems(const char* fileName, float percentEnd)
{
	fs::path path = Globals::WorkingDir;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::LogAll("Couldn't open file %s", path.generic_string().c_str());
		return;
	}

	json j;
	file >> j;

	float step = (percentEnd - LoadProgress->percentDone) / j.size();
	for (auto item : j) {

		ItemInfo* info = new ItemInfo();
		info->movementSpeed        = item["movementSpeed"].get<float>();
		info->health               = item["health"].get<float>();
		info->crit                 = item["crit"].get<float>();
		info->abilityPower         = item["abilityPower"].get<float>();
		info->mana                 = item["mana"].get<float>();
		info->armour               = item["armour"].get<float>();
		info->magicResist          = item["magicResist"].get<float>();
		info->physicalDamage       = item["physicalDamage"].get<float>();
		info->attackSpeed          = item["attackSpeed"].get<float>();
		info->lifeSteal            = item["lifeSteal"].get<float>();
		info->hpRegen              = item["hpRegen"].get<float>();
		info->movementSpeedPercent = item["movementSpeedPercent"].get<float>();
		info->cost                 = item["cost"].get<float>();
		info->id                   = item["id"].get<int>();

		LoadProgress->percentDone += step;
		Items[info->id] = info;
	}
}

void GameData::LoadUnits(const char* fileName, float percentEnd)
{
	fs::path path = Globals::WorkingDir;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::LogAll("Couldn't open file %s", path.generic_string().c_str());
		return;
	}

	json j;
	file >> j;

	float step = (percentEnd - LoadProgress->percentDone) / j.size();
	for (auto unitObj : j) {

		UnitInfo* unit = new UnitInfo();
		std::string strName = unitObj["name"].get<std::string>();

		unit->acquisitionRange        = unitObj["acquisitionRange"].get<float>();
		unit->attackSpeedRatio        = unitObj["attackSpeedRatio"].get<float>();
		unit->baseAttackRange         = unitObj["attackRange"].get<float>();
		unit->baseAttackSpeed         = unitObj["attackSpeed"].get<float>();
		unit->baseMovementSpeed       = unitObj["baseMoveSpeed"].get<float>();
		unit->basicAttackMissileSpeed = unitObj["basicAtkMissileSpeed"].get<float>();
		unit->basicAttackWindup       = unitObj["basicAtkWindup"].get<float>();
		unit->gameplayRadius          = unitObj["gameplayRadius"].get<float>();
		unit->healthBarHeight         = unitObj["healthBarHeight"].get<float>();
		unit->name                    = Strings::ToLower(strName);
		unit->pathRadius              = unitObj["pathingRadius"].get<float>();
		unit->selectionRadius         = unitObj["selectionRadius"].get<float>();

		auto tags = unitObj["tags"];
		for (auto tag : tags) {
			std::string tagStr = tag.get<std::string>();
			unit->SetTag(tagStr);
		}			

		Units[unit->name] = unit;
		LoadProgress->percentDone += step;
	}
}

bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture)
{
	// Load texture from disk
	PDIRECT3DTEXTURE9 texture;
	
	Valkyrie::DxDeviceMutex.lock();
	HRESULT hr = D3DXCreateTextureFromFileA(Valkyrie::DxDevice, filename, &texture);
	Valkyrie::DxDeviceMutex.unlock();

	if (hr != S_OK)
		return false;
	
	*out_texture = texture;
	return true;
}

void GameData::LoadImages(const char* folderName, float percentEnd)
{
	fs::path path = Globals::WorkingDir;
	path.append(FolderData).append(folderName);

	int nrFiles = std::distance(fs::directory_iterator(path), fs::directory_iterator());
	float step = (percentEnd - LoadProgress->percentDone) / nrFiles;

	std::string folder = path.string();
	WIN32_FIND_DATAA findData;
	HANDLE hFind;

	// std::filesystem has some bugs for ascii paths so we use winapi
	hFind = FindFirstFileA((folder + "\\*.png").c_str(), &findData);
	do {
		if (hFind != INVALID_HANDLE_VALUE) {
			std::string filePath = folder + "/" + findData.cFileName;

			PDIRECT3DTEXTURE9 image = NULL;
			if (!LoadTextureFromFile(filePath.c_str(), &image))
				Logger::LogAll("Failed to load %s", filePath.c_str());
			else {
				std::string fileName(findData.cFileName);
				fileName.erase(fileName.find(".png"), 4);
				Images[Strings::ToLower(fileName)] = image;
			}

			LoadProgress->percentDone += step;
		}
	} while (FindNextFileA(hFind, &findData));
}
