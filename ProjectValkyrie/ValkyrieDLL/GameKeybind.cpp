#include "GameKeybind.h"

#include "Paths.h"
#include <fstream>
#include <windows.h>
#include <string>

HKey GameKeybind::CastSpellQ;
HKey GameKeybind::CastSpellW;
HKey GameKeybind::CastSpellE;
HKey GameKeybind::CastSpellR;
HKey GameKeybind::CastSpellD;
HKey GameKeybind::CastSpellF;

HKey GameKeybind::UseItem1;
HKey GameKeybind::UseItem2;
HKey GameKeybind::UseItem3;
HKey GameKeybind::UseItem4;
HKey GameKeybind::UseItem5;
HKey GameKeybind::UseItem6;
HKey GameKeybind::UseItemTrinket;

HKey GameKeybind::PingNormal;
HKey GameKeybind::PingWarn;
HKey GameKeybind::PingDanger;
HKey GameKeybind::PingVision;
HKey GameKeybind::PingMia;
HKey GameKeybind::PingOmw;
HKey GameKeybind::PingAssist;
HKey GameKeybind::Recall;
HKey GameKeybind::TargetChampionsOnly;

std::map<std::string, std::string> GameKeybind::gameKeyCfgs;
std::map<std::string, HKey>        GameKeybind::keyConvert = {
	{"[q]", HKey::Q},
	{"[w]", HKey::W},
	{"[e]", HKey::E},
	{"[r]", HKey::R},
	{"[t]", HKey::T},
	{"[y]", HKey::Y},
	{"[u]", HKey::U},
	{"[i]", HKey::I},
	{"[o]", HKey::O},
	{"[p]", HKey::P},
	{"[a]", HKey::A},
	{"[s]", HKey::S},
	{"[d]", HKey::D},
	{"[f]", HKey::F},
	{"[g]", HKey::G},
	{"[h]", HKey::H},
	{"[j]", HKey::J},
	{"[k]", HKey::K},
	{"[l]", HKey::L},
	{"[z]", HKey::Z},
	{"[x]", HKey::X},
	{"[c]", HKey::C},
	{"[v]", HKey::V},
	{"[b]", HKey::B},
	{"[n]", HKey::N},
	{"[m]", HKey::M},

	{"[1]", HKey::N_1},
	{"[2]", HKey::N_2},
	{"[3]", HKey::N_3},
	{"[4]", HKey::N_4},
	{"[5]", HKey::N_5},
	{"[6]", HKey::N_6},
	{"[7]", HKey::N_7},
	{"[8]", HKey::N_8},
	{"[9]", HKey::N_9},
	{"[0]", HKey::N_0},

	{"[Space]",     HKey::SPACE},
	{"[`]",         HKey::TILDE},
	{"[Semicolon]", HKey::SEMICOLON},
	{"[']",         HKey::SINGLE_QUOTE},
	{"[,]",         HKey::COMMA},
	{"[.]",         HKey::DOT},
	{"[[]",         HKey::LBRACKET},
	{"[]]",         HKey::RBRACKET},
	{"[-]",         HKey::MINUS},
	{"[=]",         HKey::EQUAL},
};

void GameKeybind::InitFromGameConfigs()
{
	CHAR fileName[MAX_PATH];
	GetModuleFileNameA(NULL, fileName, MAX_PATH);

	std::string pathExe(fileName);
	int pos = pathExe.rfind("Game");
	if (pos == std::string::npos) {
		Logger::Warn("GameKeybind: Could not find 'Game' str in %s", pathExe.c_str());
		return;
	}

	std::string configFile(pathExe.substr(0, pos));
	configFile.append("\\Config\\input.ini");

	if (!Paths::FileExists(configFile)) {
		Logger::Warn("GameKeybind: Could not find config file %s", configFile.c_str());
		return;
	}

	std::ifstream file(configFile);
	std::string line;
	while (std::getline(file, line)){
		size_t eqPos = line.find("=");
		if (eqPos == std::string::npos)
			continue;

		gameKeyCfgs[line.substr(0, eqPos)] = line.substr(eqPos + 1, line.size());
	}

	CastSpellQ     = GetKeyForConfig("evtCastSpell1", HKey::Q);
	CastSpellW     = GetKeyForConfig("evtCastSpell2", HKey::W);
	CastSpellE     = GetKeyForConfig("evtCastSpell3", HKey::E);
	CastSpellR     = GetKeyForConfig("evtCastSpell4", HKey::R);
	CastSpellD     = GetKeyForConfig("evtCastAvatarSpell1", HKey::D);
	CastSpellF     = GetKeyForConfig("evtCastAvatarSpell2", HKey::F);

	UseItem1       = GetKeyForConfig("evtUseItem1", HKey::N_1);
	UseItem2       = GetKeyForConfig("evtUseItem2", HKey::N_2);
	UseItem3       = GetKeyForConfig("evtUseItem3", HKey::N_3);
	UseItem4       = GetKeyForConfig("evtUseItem4", HKey::N_5);
	UseItem5       = GetKeyForConfig("evtUseItem5", HKey::N_6);
	UseItem6       = GetKeyForConfig("evtUseItem6", HKey::N_7);
	UseItemTrinket = GetKeyForConfig("evtUseVisionItem", HKey::N_4);

	PingNormal          = GetKeyForConfig("evntPlayerPingCursor");
	PingWarn            = GetKeyForConfig("evntPlayerPingCursorDanger");
	PingDanger          = GetKeyForConfig("evtPlayerPingRadialDanger");
	PingVision          = GetKeyForConfig("evtPlayerPingAreaIsWarded");
	PingMia             = GetKeyForConfig("evtPlayerPingMIA");
	PingOmw             = GetKeyForConfig("evtPlayerPingOMW");
	PingAssist          = GetKeyForConfig("evtPlayerPingComeHere");
	Recall              = GetKeyForConfig("evtUseItem7", HKey::B);
	TargetChampionsOnly = GetKeyForConfig("evtChampionOnly");
}

HKey GameKeybind::GetKeyForConfig(std::string cfg, HKey defaultKey)
{
	auto findCfg = gameKeyCfgs.find(cfg);
	if (findCfg == gameKeyCfgs.end()) {
		Logger::Warn("Did not find game cfg %s", cfg.c_str());
		return defaultKey;
	}

	auto findKey = keyConvert.find(findCfg->second);
	if (findKey == keyConvert.end()) {
		Logger::Warn("Unreconizable game key cfg %s for %s", findCfg->second.c_str(), cfg.c_str());
		return defaultKey;
	}

	return findKey->second;
}
