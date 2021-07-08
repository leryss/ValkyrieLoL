#include "Offsets.h"

Offsets::Offsets() {};

std::string Offsets::GameVersion             = "11.14";

int Offsets::ObjectManager                   = 0x0171f818;
int Offsets::Renderer                        = 0x02fe271c;
int Offsets::ViewMatrix                      = 0x02fdf8b8;
int Offsets::MinimapObject                   = 0x02fbc2d0;
int Offsets::LocalPlayer                     = 0x02fbbb60;
int Offsets::GameTime                        = 0x02fb3f24;
int Offsets::Chat                            = 0x02fbc3a4;
int Offsets::HudInstance                     = 0x0171f854;
int Offsets::UnderMouseObject                = 0x02FB0CC0;
int Offsets::FnCharacterDataStackUpdate      = 0x000f34b0;
int Offsets::CharacterDataStack              = 0x00002bb0;

int Offsets::HudInstanceMouseWorldPosition   = 0x11c;
int Offsets::ChatIsOpen                      = 0x684;

int Offsets::ProjectionMatrix                = ViewMatrix + 16*sizeof(float);
int Offsets::PlayerName                      = 0x6C;

int Offsets::ObjIndex                        = 0x20;
int Offsets::ObjTeam                         = 0x4C;
int Offsets::ObjNetworkID                    = 0xCC;
int Offsets::ObjPos                          = 0x1d8;
int Offsets::ObjMissileSpellCast             = 0x250;
int Offsets::ObjExpiry                       = 0x298; 
int Offsets::ObjVisibility                   = 0x270;
int Offsets::ObjSpawnCount                   = 0x218;
int Offsets::ObjMana                         = 0x298;
int Offsets::ObjInvulnerable                 = 0x3D0;
int Offsets::ObjTargetable                   = 0xD00;
int Offsets::ObjRecallState                  = 0xD8C;
int Offsets::ObjHealth                       = 0xD98;
int Offsets::ObjMaxHealth                    = 0xDA8;
int Offsets::ObjMaxMana                      = 0x2A8;

int Offsets::ObjAbilityHaste                 = 0x10F4;
int Offsets::ObjLethality                    = 0x11E0;
int Offsets::ObjArmor                        = 0x12CC;
int Offsets::ObjBonusArmor                   = 0x12D0;
int Offsets::ObjMagicRes                     = 0x12D4;
int Offsets::ObjBonusMagicRes                = 0x12D8;
int Offsets::ObjBaseAtk                      = 0x12A4;
int Offsets::ObjBonusAtk                     = 0x121C;
int Offsets::ObjMoveSpeed                    = 0x12E4;
int Offsets::ObjCrit                         = 0x12C8;
int Offsets::ObjCritMulti                    = 0x12B8;
int Offsets::ObjAbilityPower                 = 0x122C;
int Offsets::ObjAtkSpeedMulti                = 0x12A0;
int Offsets::ObjAtkRange                     = 0x12EC;
int Offsets::ObjMagicPen                     = 0x11C4;
int Offsets::ObjMagicPenMulti                = 0x11CC; /// 1.0 when to percent magic pen is applied othrwise its below 1.0 depending on the percent applied
int Offsets::ObjAdditionalApMulti            = 0x1230; /// I use this for rabadon, its 0.35 when rabadon is in inventory

int Offsets::ObjDirection                    = 0x1BD8;

int Offsets::ObjName                         = 0x2bbc;
int Offsets::ObjTransformation               = Offsets::ObjName - 0xC;
int Offsets::ObjLvl                          = 0x3344;

int Offsets::ObjBuffManager                  = 0x21A4;
int Offsets::BuffManagerEntriesArray         = 0x10;

int Offsets::ObjSpellBook                    = 0x2358;
int Offsets::SpellBookActiveSpellCast        = 0x20;
int Offsets::SpellBookCastableMask           = 0x38;
int Offsets::SpellBookSpellSlots             = 0x478;

int Offsets::SpellCastSpellInfo              = 0x8;
int Offsets::SpellCastStartTime	             = 0x14c;
int Offsets::SpellCastStartTimeAlt           = 0x13C;
int Offsets::SpellCastCastTime               = 0xc8;
int Offsets::SpellCastStart                  = 0x80;
int Offsets::SpellCastEnd                    = 0x8C;
int Offsets::SpellCastSrcIdx                 = 0x68;
int Offsets::SpellCastDestIdx                = 0xBC;

int Offsets::ObjItemList                     = 0x3390;
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
int Offsets::SpellDataSpellName              = 0x6C;
int Offsets::SpellDataMissileName            = 0x6C;
int Offsets::SpellDataManaArray              = 0x52C;
										    
int Offsets::ObjectMapCount                  = 0x2C;
int Offsets::ObjectMapRoot                   = 0x28;
int Offsets::ObjectMapNodeNetId              = 0x10;
int Offsets::ObjectMapNodeObject             = 0x14;

int Offsets::MinimapObjectHud                = 0x14;
int Offsets::MinimapHudPos                   = 0x60;
int Offsets::MinimapHudSize                  = 0x68;          // has values between (191, 191) and (383, 383)
										    
int Offsets::CharacterDataStackSkinId        = 0x18;

int Offsets::ObjVTableGetAiManager           = 149;
int Offsets::AiManagerStartPath              = 0x1e4;
int Offsets::AiManagerEndPath                = 0x1e8;
int Offsets::AiManagerTargetPosition         = 0x10;
int Offsets::AiManagerIsMoving               = 0x1c0;
int Offsets::AiManagerIsDashing              = 0x3c0;
int Offsets::AiManagerCurrentSegment         = 0x1c4;
int Offsets::AiManagerDashSpeed              = 0x1f8;
int Offsets::AiManagerVelocity               = 0x2f0;