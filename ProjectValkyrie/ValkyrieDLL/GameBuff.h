#pragma once

#include "MemoryReadable.h"

enum GameBuffType {
	BuffTypeUnknown       = 0,
	BuffTypeInternal      = 1,
	BuffTypeAura             ,
	BuffTypeCombatBuff       ,
	BuffTypeCombatDebuff     ,
	BuffTypeSpellShield      ,
	BuffTypeStun             ,
	BuffTypeInvisibility     ,
	BuffTypeSilence          ,
	BuffTypeTaunt            ,
	BuffTypePolymorph        ,
	BuffTypeSlow             ,
	BuffTypeSnare            ,
	BuffTypeDamage           ,
	BuffTypeHeal             ,
	BuffTypeHaste            ,
	BuffTypeSpellImmunity    ,
	BuffTypePhysicalImmunity ,
	BuffTypeInvulnerability  ,
	BuffTypeSleep            ,
	BuffTypeNearSight        ,
	BuffTypeFrenzy           ,
	BuffTypeFear             ,
	BuffTypeCharm            ,
	BuffTypePoison           ,
	BuffTypeSuppression      ,
	BuffTypeBlind            ,
	BuffTypeCounter          ,
	BuffTypeShred            ,
	BuffTypeFlee             ,
	BuffTypeKnockup          ,
	BuffTypeKnockback        ,
	BuffTypeDisarm           ,
};

class GameBuff {

public:
	void   ImGuiDraw();

public:
	int          address;
	GameBuffType type = BuffTypeUnknown;
	float        startTime;
	float        endTime;
	int          count;
	float        value;
	std::string  name;
};

struct Buff {

	char pad1[0x8];
	char buffName[120];
};
	
struct BuffEntry {

	char  pad1[0x4];
	char  type;
	char  pad2[0x3];
	Buff* buff;
	float startTime;
	float endTime;
	char  pad3[0xC];
	int   buffNodeStart;
	int   buffNodeEnd; //0x28
	char  pad4[0x8];
	float valueAlt;
	char  pad5[0x44];
	int   value;

	int GetCount() const;
};

struct BuffEntryWrap {
	BuffEntry* entry;
	char pad[4];
};