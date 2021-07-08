#include "GameReader.h"
#include "GameData.h"

#include "Strings.h"
#include "Valkyrie.h"
#include "json/json.h"
#include "Paths.h"

#include "d3dx9tex.h"
#include <filesystem>
#include <thread>

#include <windows.h>
#include "miniz/miniz.h"

using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

AsyncTaskPool*                    GameData::TaskPool = AsyncTaskPool::Get();
const char*                       GameData::FolderData = "data";
bool*                             GameData::WallMask = new bool[512 * 512];

bool                              GameData::EssentialsLoaded = false;
bool                              GameData::EverythingLoaded = false;

std::map<std::string, UnitInfo*>              GameData::Units;
std::map<std::string, SpellInfo*>             GameData::Spells;
std::map<std::string, PDIRECT3DTEXTURE9>      GameData::Images;
std::map<std::string, std::vector<SkinInfo*>> GameData::Skins;
std::map<int, ItemInfo*>                      GameData::Items;

void GameData::LoadEverything()
{
	TaskPool->DispatchTask(
		"Load Essentials",
		std::shared_ptr<GameDataEssentialsLoad>(new GameDataEssentialsLoad()),

		[](std::shared_ptr<AsyncTask> response) {
			EssentialsLoaded = true;
			TaskPool->DispatchTask(
				"Load Extras", std::shared_ptr<GameDataImagesLoad>(new GameDataImagesLoad()), [](std::shared_ptr<AsyncTask> response) {
					EverythingLoaded = true;
				}
			);
		}
	);
}

void GameData::LoadEssentials()
{
	EssentialsLoaded = false;
	TaskPool->DispatchTask(
		"Load Essentials",
		std::shared_ptr<GameDataEssentialsLoad>(new GameDataEssentialsLoad()),

		[](std::shared_ptr<AsyncTask> response) {
			EssentialsLoaded = true;
		}
	);
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

PDIRECT3DTEXTURE9 GameData::GetImage(const char* str)
{
	std::string key(str);
	auto find = Images.find(key);
	return (find == Images.end() ? nullptr : find->second);
}

std::vector<SkinInfo*>& GameData::GetSkins(std::string & name)
{
	static std::vector<SkinInfo*> EmptyVec;

	auto find = Skins.find(name);
	return (find == Skins.end() ? EmptyVec : find->second);
}

bool GameData::IsWallAt(const Vector3 & worldPos)
{
	/// TODO: Make wall detection for all maps
	if (Valkyrie::CurrentGameState->map.type != GAME_MAP_SRU)
		return false;

	static const float ratio = 512.f / 14900.f;
	int x = (int)ceil(worldPos.x * ratio);
	int y = (int)ceil(worldPos.z * ratio);

	if (x < 0 || x >= 512 || y < 0 || y >= 512)
		return false;

	return WallMask[x*512 + y];
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

void GameData::LoadWallMask(const char * filename, float & percentValue, float percentEnd)
{
	fs::path path = Paths::Root;
	path.append(FolderData).append(filename);
	std::ifstream file(path.generic_string().c_str(), std::ifstream::binary);

	file.read((char*)WallMask, 512 * 512);

	percentValue = percentEnd;
}

void GameData::LoadSpells(const char* fileName, float& percentValue, float percentEnd)
{
	fs::path path = Paths::Root;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::Error("Couldn't open file %s", path.generic_string().c_str());
		return;
	}

	json j;
	file >> j;

	float step = (percentEnd - percentValue) / j.size();
	for (auto spell : j) {
		SpellInfo* info = new SpellInfo();

		std::string strIcon = spell["icon"].get<std::string>();
		std::string strName = spell["name"].get<std::string>();

		info->castTime         = spell["castTime"].get<float>();
		info->height           = spell["height"].get<float>();
		info->icon             = Strings::ToLower(strIcon);
		info->name             = Strings::ToLower(strName);
		info->width            = spell["width"].get<float>();
		info->castRange        = spell["castRange"].get<float>();
		info->castRadius       = spell["castRadius"].get<float>();
		info->castConeAngle    = spell["castConeAngle"].get<float>();
		info->castConeDistance = spell["castConeDistance"].get<float>();
		info->speed            = spell["speed"].get<float>();
		info->travelTime       = spell["travelTime"].get<float>();
		info->delay            = spell["delay"].get<float>();
		info->parentName       = spell["parent"].get<std::string>();

		auto flags = spell["flags"];
		for (auto& flag : flags) {
			auto strFlag = flag.get<std::string>();
			info->AddFlag(strFlag);
		}
		percentValue += step;
		Spells[info->name] = info;
	}

	/// Fill parent spells
	for (auto& pair : Spells) {
		auto s1 = pair.second;
		if (!s1->parentName.empty()) {
			auto find = Spells.find(s1->parentName);
			if (find != Spells.end())
				s1->parent = find->second;
		}
	}
}

void GameData::LoadItems(const char* fileName, float& percentValue, float percentEnd)
{
	fs::path path = Paths::Root;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::Error("Couldn't open file %s", path.generic_string().c_str());
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
	fs::path path = Paths::Root;
	path.append(FolderData).append(fileName);
	std::ifstream file(path.generic_string().c_str());

	if (!file.is_open()) {
		Logger::Error("Couldn't open file %s", path.generic_string().c_str());
		return;
	}

	json j;
	file >> j;

	float step = (percentEnd - percentValue) / j.size();
	for (auto unitObj : j) {

		UnitInfo* unit = new UnitInfo();
		std::string strName = unitObj["name"].get<std::string>();
		std::string basicAtk = unitObj["basicAtk"].get<std::string>();

		unit->acquisitionRange        = unitObj["acquisitionRange"].get<float>();
		unit->attackSpeedRatio        = unitObj["attackSpeedRatio"].get<float>();
		unit->baseAttackRange         = unitObj["attackRange"].get<float>();
		unit->baseAttackSpeed         = unitObj["attackSpeed"].get<float>();
		unit->baseMovementSpeed       = unitObj["baseMoveSpeed"].get<float>();
		unit->basicAttack             = GetSpell(basicAtk);
		unit->basicAttackWindup       = unitObj["basicAtkWindup"].get<float>();
		unit->basicAttackCastTime     = unitObj["basicAtkCastTime"].get<float>();
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

bool LoadTextureFromHeap(void* heap, size_t heapSize, PDIRECT3DTEXTURE9* outTexture) {
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
	fs::path path = Paths::Root;
	path.append(FolderData).append(zipName);
	const char* zipPath = path.u8string().c_str();

	Logger::Info("Opening %s", zipPath);

	mz_zip_archive archive;
	memset(&archive, 0, sizeof(archive));
	if (!mz_zip_reader_init_file(&archive, zipPath, 0)) {
		Logger::Error("Failed to open zip %s: %s", zipName, mz_zip_get_error_string(mz_zip_get_last_error(&archive)));
		return;
	}

	int numImages = mz_zip_reader_get_num_files(&archive);
	float step = (percentEnd - percentValue) / numImages;

	for (int i = 0; i < numImages; ++i) {
		mz_zip_archive_file_stat fileStat;
		if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
			Logger::Error("Failed to get image num %d from %s", i, zipName);
			continue;
		}

		size_t imgSize = 0;
		void* imgBin = mz_zip_reader_extract_file_to_heap(&archive, fileStat.m_filename, &imgSize, 0);
		if (imgBin == NULL) {
			Logger::Error("Failed to uncompress image num %d from %s", i, zipName);
			continue;
		}

		std::string imgName = std::string(fileStat.m_filename);
		imgName.erase(imgName.size() - 4, imgName.size());

		PDIRECT3DTEXTURE9 image = NULL;
		if (!LoadTextureFromHeap(imgBin, (size_t)fileStat.m_uncomp_size, &image))
			Logger::Error("Failed to load img %s", imgName);
		else 
			Images[Strings::ToLower(imgName)] = image;

		percentValue += step;
	}

	mz_zip_reader_end(&archive);
}

void GameDataEssentialsLoad::Perform()
{
	try {
		percentDone = 0.f;
		currentStep = "Load static game data";
		GameData::LoadWallMask("WallMask.bin", percentDone, 0.1f);
		Logger::Info("Loaded wall mask");

		GameData::LoadSpells("SpellData.json", percentDone, 0.25f);
		Logger::Info("Loaded spells");

		GameData::LoadUnits("UnitData.json", percentDone, 0.6f);
		Logger::Info("Loaded units");

		GameData::LoadItems("ItemData.json", percentDone, 0.1f);
	}
	catch (std::exception& exc) {
		SetError(exc.what());
		return;
	}
	SetStatus(ASYNC_SUCCEEDED);
}

void GameDataImagesLoad::Perform()
{
	percentDone = 0.f;
	currentStep = "Load icons";
	GameData::LoadImagesFromZip("icons_spells.zip", percentDone, 0.8f);
	GameData::LoadImagesFromZip("icons_champs.zip", percentDone, 0.95f);
	GameData::LoadImagesFromZip("icons_extra.zip", percentDone, 1.f);
	Logger::Info("Loaded images");

	SetStatus(ASYNC_SUCCEEDED);
}
