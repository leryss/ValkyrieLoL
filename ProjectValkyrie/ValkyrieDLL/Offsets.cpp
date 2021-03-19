#include "Offsets.h"

Offsets::Offsets() {};

std::string Offsets::GameVersion             = "11.6";

int Offsets::ObjectManager                   = 0x016f8678;
int Offsets::Renderer                        = 0x02fba770;
int Offsets::ViewMatrix                      = 0x02fb7998;
int Offsets::MinimapObject                   = 0x02f94b0c;
int Offsets::LocalPlayer                     = 0x02f9512c;
int Offsets::GameTime                        = 0x02f8d1dc;
int Offsets::Chat                            = 0x02f951f8;
int Offsets::FnCharacterDataStackUpdate      = 0x000f3e00;

int Offsets::UnderMouseObject                = 0x2346840;
int Offsets::ProjectionMatrix                = ViewMatrix + 16*sizeof(float);

int Offsets::ChatIsOpen                      = 0x650;

int Offsets::ObjIndex                        = 0x20;
int Offsets::ObjTeam                         = 0x4C;
int Offsets::ObjMissileName                  = 0x6C;
int Offsets::ObjNetworkID                    = 0xCC;
int Offsets::ObjPos                          = 0x1d8;
int Offsets::ObjMissileSpellCast             = 0x228;
int Offsets::ObjVisibility                   = 0x270;
int Offsets::ObjSpawnCount                   = 0x284;
int Offsets::ObjMana                         = 0x298;
int Offsets::ObjRecallState                  = 0xD8C;
int Offsets::ObjHealth                       = 0xD98;
int Offsets::ObjMaxHealth                    = 0xDA8;
int Offsets::ObjArmor                        = 0x129C;
int Offsets::ObjMagicRes                     = 0x12A4;
int Offsets::ObjBaseAtk                      = 0x1274;
int Offsets::ObjBonusAtk                     = 0x11F0;
int Offsets::ObjMoveSpeed                    = 0x12B4;
int Offsets::ObjTransformation               = 0x2f78;
int Offsets::ObjName                         = 0x2F84;
int Offsets::ObjLvl                          = 0x36D4;
int Offsets::ObjExpiry                       = 0x298; 
int Offsets::ObjCrit                         = 0x1298;
int Offsets::ObjCritMulti                    = 0x1288;
int Offsets::ObjAbilityPower                 = 0x1200;
int Offsets::ObjAtkSpeedMulti                = 0x1270;
int Offsets::ObjAtkRange                     = 0x12BC;
int Offsets::ObjTargetable                   = 0xD00;
int Offsets::ObjInvulnerable                 = 0x3D0;
int Offsets::ObjIsMoving                     = 0x3638;  /// Not a bool, but its 0 when not moving and some point when its moving
int Offsets::ObjDirection                    = 0x1B80;
int Offsets::ObjLethality                    = 0x11B4;
int Offsets::ObjAbilityHaste                 = 0x10cc;
int Offsets::ObjBonusArmor                   = 0x12A0;
int Offsets::ObjBonusMagicRes                = 0x12A8;

int Offsets::ObjMagicPen                     = 0x1198; 
int Offsets::ObjMagicPenMulti                = 0x11A0; /// 1.0 when to percent magic pen is applied othrwise its below 1.0 depending on the percent applied
int Offsets::ObjAdditionalApMulti            = 0x1204; /// I use this for rabadon, its 0.35 when rabadon is in inventory

int Offsets::ObjBuffManager                  = 0x2178;
int Offsets::BuffManagerEntriesArray         = 0x10;
int Offsets::BuffEntryBuff                   = 0x8;
int Offsets::BuffEntryBuffStartTime          = 0xC;
int Offsets::BuffEntryBuffEndTime            = 0x10;
int Offsets::BuffEntryBuffCount              = 0x74;
int Offsets::BuffName                        = 0x8;
//oObjIsMoving = 363B;
//oObjDirection = 0x1B80;
//oObjActionState = 0x1054;

int Offsets::ObjSpellBook                    = 0x2720;
int Offsets::SpellBookActiveSpellCast        = 0x20;
int Offsets::SpellBookSpellSlots             = 0x478;

int Offsets::SpellCastSpellInfo              = 0x8;
int Offsets::SpellCastStartTime              = 0x544;
int Offsets::SpellCastCastTime               = 0x4C0;
int Offsets::SpellCastStart                  = 0x80;
int Offsets::SpellCastEnd                    = 0x8C;
int Offsets::SpellCastSrcIdx                 = 0x68;
int Offsets::SpellCastDestIdx                = 0xC0;

int Offsets::ObjItemList                     = 0x3720;
int Offsets::ItemListItem                    = 0xC;
int Offsets::ItemActiveName                  = 0x10;
int Offsets::ItemCharges                     = 0x24;
int Offsets::ItemInfo                        = 0x20;
int Offsets::ItemInfoId                      = 0x68;
										    
int Offsets::RendererWidth                   = 0x10;
int Offsets::RendererHeight                  = 0x14;
										    
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
										    
int Offsets::CharacterDataStack              = 0x2F78;
int Offsets::CharacterDataStackSkinId        = 0x18;