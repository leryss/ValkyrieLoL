from valkyrie import *
from helpers.targeting import TargetSelector, TargetSet

import helpers.templates as HT
from   helpers.spells import SpellRotation, RSpell, Slot, SpellCondition
		
samira = HT.ChampionScript(
	passive_trigger   = HT.Enabler.default(),
	combat_distance   = 950,
	passive_distance  = 950,
	
	combat_rotation = SpellRotation([
		RSpell(Slot.R), 
		RSpell(Slot.Q), 
		RSpell(Slot.W, HT.ConditionIncomingMissiles(0.75)), 
		RSpell(Slot.E, HT.MixedConditions([
			HT.ConditionTargetOutsideTower(),
			HT.MixedConditions([
				HT.ConditionDistanceToTarget(400.0, 1000.0), 
				HT.ConditionTargetHPBelow(40)
			], HT.MixedConditions.Any)
			
		], HT.MixedConditions.All))
	]),

	passive_rotation = SpellRotation([
		RSpell(Slot.Q)
	])
)

leap_under_tower = False
leap_gap_closer  = True
leap_below_hp    = 30
	
def valkyrie_menu(ctx) :
	ui = ctx.ui					 
	samira.ui(ctx)
	
def valkyrie_on_load(ctx) :	 
	global samira
	cfg = ctx.cfg				 
	
	samira = HT.ChampionScript.from_str(cfg.get_str('samira', str(samira)))
	
def valkyrie_on_save(ctx) :	
	cfg = ctx.cfg				 
	cfg.set_str('samira', str(samira))
	
def valkyrie_exec(ctx) :
	
	if ctx.player.dead:
		return
	samira.exec(ctx)