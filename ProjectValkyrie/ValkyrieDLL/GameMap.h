#pragma once

enum MapType {
	GAME_MAP_SRU = 0x7273, /// 'sr'
	GAME_MAP_HA = 0x6168, /// 'ha'
};

class GameMap {

public:
	MapType type;
};