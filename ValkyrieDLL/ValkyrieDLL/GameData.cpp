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
#include "miniz/miniz.h"

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

void GameData::ImGuiDrawLoader()
{
	ImGui::SetNextWindowSize(ImVec2(200, 200));
	ImGui::Begin("Valkyrie Loader", NULL, ImGuiWindowFlags_NoResize);
	ImGui::Text("Spell Database");
	ImGui::ProgressBar(LoadProgress->spellLoadPercent);

	ImGui::Text("Unit Database");
	ImGui::ProgressBar(LoadProgress->unitsLoadPercent);

	ImGui::Text("Items Database");
	ImGui::ProgressBar(LoadProgress->itemsLoadPercent);

	ImGui::Text("Image Database");
	ImGui::ProgressBar(LoadProgress->imagesLoadPercent);
	ImGui::End();
}

void GameData::ImGuiDrawObjects()
{
	static char filterBuf[200];
	ImGui::InputText("Filter", filterBuf, 200);

	std::string filter(filterBuf);
	if (ImGui::CollapsingHeader("Units")) {
		for (auto& pair : Units) {
			if (pair.first.find(filter) != std::string::npos) {
				if (ImGui::TreeNode(pair.first.c_str())) {
					pair.second->ImGuiDraw();
					ImGui::TreePop();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Spells")) {
		for (auto& pair : Spells) {
			if (pair.first.find(filter) != std::string::npos) {
				if (ImGui::TreeNode(pair.first.c_str())) {
					pair.second->ImGuiDraw();
					ImGui::TreePop();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Images")) {
		for (auto& pair : Images) {
			if (pair.first.find(filter) != std::string::npos) {
				if (ImGui::TreeNode(pair.first.c_str())) {
					ImGui::Image(pair.second, ImVec2(48, 48));
					ImGui::TreePop();
				}
			}
		}
	}
}

void GameData::Load()
{
	Valkyrie::WaitForOverlayToInit();

	LoadSpells("SpellData.json",       LoadProgress->spellLoadPercent, 0.9f);
	LoadSpells("SpellDataCustom.json", LoadProgress->spellLoadPercent, 1.f);
	Logger::LogAll("Loaded %zu spells", Spells.size());

	LoadUnits("UnitData.json", LoadProgress->unitsLoadPercent, 1.f);
	Logger::LogAll("Loaded %zu units", Units.size());

	LoadItems("ItemData.json", LoadProgress->itemsLoadPercent, 1.f);
	Logger::LogAll("Loaded %zu items", Items.size());

	LoadProgress->essentialsLoaded = true;

	LoadImagesFromZip("icons_spells.zip", LoadProgress->imagesLoadPercent, 0.8f);
	LoadImagesFromZip("icons_champs.zip", LoadProgress->imagesLoadPercent, 0.95f);
	LoadImagesFromZip("icons_extra.zip",  LoadProgress->imagesLoadPercent, 1.f);
	Logger::LogAll("Loaded %zu images", Images.size());

	Logger::LogAll("Static data loading complete");
	LoadProgress->allLoaded = true;
}

void GameData::LoadSpells(const char* fileName, float& percentValue, float percentEnd)
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

	float step = (percentEnd - percentValue) / j.size();
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

		percentValue += step;
		Spells[info->name] = info;
	}
}

void GameData::LoadItems(const char* fileName, float& percentValue, float percentEnd)
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

	float step = (percentEnd - percentValue) / j.size();
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

		percentValue += step;
		Items[info->id] = info;
	}
}

void GameData::LoadUnits(const char* fileName, float& percentValue, float percentEnd)
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

	float step = (percentEnd - percentValue) / j.size();
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
		percentValue += step;
	}
}

bool LoadTextureFromHeap(void* heap, int heapSize, PDIRECT3DTEXTURE9* outTexture) {
	PDIRECT3DTEXTURE9 texture;

	Valkyrie::DxDeviceMutex.lock();
	HRESULT hr = D3DXCreateTextureFromFileInMemory(Valkyrie::DxDevice, heap, heapSize, &texture);
	Valkyrie::DxDeviceMutex.unlock();
	
	if (hr != S_OK)
		return false;

	*outTexture = texture;
	return true;
}


void GameData::LoadImagesFromZip(const char* zipName, float& percentValue, float percentEnd) {
	fs::path path = Globals::WorkingDir;
	path.append(FolderData).append(zipName);
	const char* zipPath = path.u8string().c_str();

	Logger::LogAll("Opening %s", zipPath);

	mz_zip_archive archive;
	memset(&archive, 0, sizeof(archive));
	if (!mz_zip_reader_init_file(&archive, zipPath, 0)) {
		Logger::LogAll("Failed to load %s", zipName);
		return;
	}

	int numImages = mz_zip_reader_get_num_files(&archive);
	float step = (percentEnd - percentValue) / numImages;

	for (int i = 0; i < numImages; ++i) {
		mz_zip_archive_file_stat fileStat;
		if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
			Logger::LogAll("Failed to get image num %d from %s", i, zipName);
			continue;
		}

		size_t imgSize = 0;
		void* imgBin = mz_zip_reader_extract_file_to_heap(&archive, fileStat.m_filename, &imgSize, 0);
		if (imgBin == NULL) {
			Logger::LogAll("Failed to uncompress image num %d from %s", i, zipName);
			continue;
		}

		std::string imgName = std::string(fileStat.m_filename);
		imgName.erase(imgName.size() - 4, imgName.size());

		PDIRECT3DTEXTURE9 image = NULL;
		if (!LoadTextureFromHeap(imgBin, fileStat.m_uncomp_size, &image))
			Logger::LogAll("Failed to load %s", imgName);
		else 
			Images[Strings::ToLower(imgName)] = image;

		percentValue += step;
	}

	mz_zip_reader_end(&archive);
}