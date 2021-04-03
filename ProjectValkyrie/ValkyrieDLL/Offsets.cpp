#include "Offsets.h"

Offsets::Offsets() {};

std::string Offsets::GameVersion             = "11.7";

int Offsets::ObjectManager                   = 0x016d85b8;
int Offsets::Renderer                        = 0x02f9add0;
int Offsets::ViewMatrix                      = 0x02f97ff0;
int Offsets::MinimapObject                   = 0x02f74b38;
int Offsets::LocalPlayer                     = 0x02f7513c;
int Offsets::GameTime                        = 0x02f6d134;
int Offsets::Chat                            = 0x02f75208;
int Offsets::FnCharacterDataStackUpdate      = 0x000ee560;

int Offsets::UnderMouseObject                = 0x2346840;
int Offsets::ProjectionMatrix                = ViewMatrix + 16*sizeof(float);

int Offsets::ChatIsOpen                      = 0x650;

int Offsets::ObjIndex                        = 0x20;
int Offsets::ObjTeam                         = 0x4C;
int Offsets::ObjNetworkID                    = 0xCC;
int Offsets::ObjPos                          = 0x1d8;
int Offsets::ObjMissileSpellCast             = 0x250;
int Offsets::ObjExpiry                       = 0x298; 
int Offsets::ObjVisibility                   = 0x270;
int Offsets::ObjSpawnCount                   = 0x284;
int Offsets::ObjMana                         = 0x298;
int Offsets::ObjInvulnerable                 = 0x3D0;
int Offsets::ObjTargetable                   = 0xD00;
int Offsets::ObjRecallState                  = 0xD8C;
int Offsets::ObjHealth                       = 0xD98;
int Offsets::ObjMaxHealth                    = 0xDA8;

int Offsets::ObjAbilityHaste                 = 0x10F4;
int Offsets::ObjLethality                    = 0x11DC;
int Offsets::ObjArmor                        = 0x12C4;
int Offsets::ObjBonusArmor                   = 0x12C8;
int Offsets::ObjMagicRes                     = 0x12CC;
int Offsets::ObjBonusMagicRes                = 0x12D0;
int Offsets::ObjBaseAtk                      = 0x129C;
int Offsets::ObjBonusAtk                     = 0x1218;
int Offsets::ObjMoveSpeed                    = 0x12DC;
int Offsets::ObjCrit                         = 0x12C0;
int Offsets::ObjCritMulti                    = 0x12B0;
int Offsets::ObjAbilityPower                 = 0x1228;
int Offsets::ObjAtkSpeedMulti                = 0x1298;
int Offsets::ObjAtkRange                     = 0x12E4;
int Offsets::ObjMagicPen                     = 0x11C0;
int Offsets::ObjMagicPenMulti                = 0x11C8; /// 1.0 when to percent magic pen is applied othrwise its below 1.0 depending on the percent applied
int Offsets::ObjAdditionalApMulti            = 0x122C; /// I use this for rabadon, its 0.35 when rabadon is in inventory

int Offsets::ObjDirection                    = 0x1B88;

int Offsets::ObjTransformation               = 0x2f80;
int Offsets::ObjName                         = 0x2F8C;
int Offsets::ObjLvl                          = 0x36DC;

int Offsets::ObjBuffManager                  = 0x217C;
int Offsets::BuffManagerEntriesArray         = 0x10;
int Offsets::BuffEntryBuff                   = 0x8;
int Offsets::BuffEntryBuffStartTime          = 0xC;
int Offsets::BuffEntryBuffEndTime            = 0x10;
int Offsets::BuffEntryBuffCount              = 0x74;
int Offsets::BuffName                        = 0x8;
int Offsets::BuffEntryBuffNodeStart          = 0x20;
int Offsets::BuffEntryBuffNodeCurrent        = 0x24;
//oObjActionState = 0x1054;

int Offsets::ObjSpellBook                    = 0x2728;
int Offsets::SpellBookActiveSpellCast        = 0x20;
int Offsets::SpellBookCastableMask           = 0x38;
int Offsets::SpellBookSpellSlots             = 0x478;

int Offsets::SpellCastSpellInfo              = 0x8;
int Offsets::SpellCastStartTime              = 0x544;
int Offsets::SpellCastCastTime               = 0x4C0;
int Offsets::SpellCastStart                  = 0x80;
int Offsets::SpellCastEnd                    = 0x8C;
int Offsets::SpellCastSrcIdx                 = 0x68;
int Offsets::SpellCastDestIdx                = 0xC0;

int Offsets::ObjItemList                     = 0x3728;
int Offsets::ItemListItem                    = 0xC;
int Offsets::ItemActiveName                  = 0x10;
int Offsets::ItemCharges                     = 0x24;
int Offsets::ItemInfo                        = 0x20;
int Offsets::ItemInfoId                      = 0x68;
										    
int Offsets::RendererWidth                   = 0xC;
int Offsets::RendererHeight                  = 0x10;
										    
int Offsets::SpellSlotLevel                  = 0x20;
int Offsets::SpellSlotTime                   = 0x28;
int Offsets::SpellSlotCharges                = 0x58;
int Offsets::SpellSlotTimeCharge             = 0x64;
int Offsets::SpellSlotValue                  = 0x94;
int Offsets::SpellSlotSpellInfo              = 0x13C;
int Offsets::SpellInfoSpellData              = 0x44;
int Offsets::SpellDataSpellName              = 0x64;
int Offsets::SpellDataMissileName            = 0x64;
int Offsets::SpellDataManaArray              = 0x524;
										    
int Offsets::ObjectMapCount                  = 0x2C;
int Offsets::ObjectMapRoot                   = 0x28;
int Offsets::ObjectMapNodeNetId              = 0x10;
int Offsets::ObjectMapNodeObject             = 0x14;

int Offsets::MinimapObjectHud                = 0x88;
int Offsets::MinimapHudPos                   = 0x60;
int Offsets::MinimapHudSize                  = 0x68;          // has values between (191, 191) and (383, 383)
										    
int Offsets::CharacterDataStack              = 0x2F80;
int Offsets::CharacterDataStackSkinId        = 0x18;

int Offsets::ObjVTableGetAiManager           = 148;
int Offsets::AiManagerStartPath              = 0x1bc;
int Offsets::AiManagerEndPath                = 0x1c0;
int Offsets::AiManagerTargetPosition         = 0x10;
int Offsets::AiManagerIsMoving               = 0x198;
int Offsets::AiManagerIsDashing              = 0x398;
int Offsets::AiManagerCurrentSegment         = 0x19C;
int Offsets::AiManagerDashSpeed              = 0x1D0;