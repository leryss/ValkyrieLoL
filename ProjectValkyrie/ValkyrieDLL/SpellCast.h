#pragma once
#include "MemoryReadable.h"
#include "Vector.h"
#include "SpellInfo.h"
#include "Collidable.h"
#include <boost/python.hpp>

using namespace boost::python;

class SpellCast : public MemoryReadable, public Collidable {

public:
	virtual void ReadFromBaseAddress(int address) override;
	void         ImGuiDraw();
	object       GetStaticData();
	float        RemainingCastTime() const;

public:

	float       timeBegin;
	float       castTime;
	Vector3     start;
	Vector3     end;
	Vector3     dir;
	short       srcIndex;
	short       destIndex;

	std::string name;
	SpellInfo*  staticData;
};