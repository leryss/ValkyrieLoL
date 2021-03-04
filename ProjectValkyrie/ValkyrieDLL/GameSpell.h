#pragma once
#include "Memory.h"
#include "Offsets.h"
#include "MemoryReadable.h"
#include "SpellInfo.h"
#include "imgui/imgui.h"
#include "HKey.h"

#include <chrono>
#include <boost/python.hpp>

using namespace boost::python;
using namespace std::chrono;

class GameSpell : public MemoryReadable {

public:
	void        ReadFromBaseAddress(int addr);
	void        ImGuiDraw();

	float       GetRemainingCooldown();
	object      GetStaticData();
public:
	std::string name;
	int         lvl;
	float       readyAt;
	float       readyAtCharge;
	int         charges;
	float       value;
	HKey        castKey;

	SpellInfo*  staticData;

	time_point<steady_clock> lastCastTimestamp;
};