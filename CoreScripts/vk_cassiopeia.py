from valkyrie import *			
from time import time
from helpers.targeting import TargetSelector, TargetSet
from helpers.flags import Orbwalker
from helpers.damages import calculate_raw_spell_dmg

import helpers.templates as HT
from helpers.spells import SpellRotation, RSpell, Slot, SpellCondition

cassiopeia = HT.ChampionScript(
	passive_trigger   = HT.Enabler.default(),
	combat_distance   = 900,
	passive_distance  = 900,
	
	combat_rotation = SpellRotation([
		RSpell(Slot.R, HT.ConditionInFrontOfTarget()), 
		RSpell(Slot.W), 
		RSpell(Slot.Q), 
		RSpell(Slot.E)
	]),

	passive_rotation = SpellRotation([
		RSpell(Slot.Q), 
		RSpell(Slot.E, HT.ConditionTargetPoisoned())
	])
)

last_hit_e = True

def valkyrie_menu(ctx) :		 
	global cassiopeia, last_hit_e
	ui = ctx.ui					 
	
	cassiopeia.ui(ctx)
	last_hit_e = ui.checkbox('Auto last hit with E (No prediction)', last_hit_e)
	
def valkyrie_on_load(ctx) :	 
	global cassiopeia, last_hit_e
	cfg = ctx.cfg				 
	
	cassiopeia = HT.ChampionScript.from_str(cfg.get_str('cassiopeia', str(cassiopeia)))
	last_hit_e = cfg.get_bool('last_hit_e', False)
	
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	
	cfg.set_bool('last_hit_e', last_hit_e)
	cfg.set_str('cassiopeia', str(cassiopeia))
	
def valkyrie_exec(ctx) :	     
	
	if ctx.player.dead:
		return
		
	cassiopeia.exec(ctx)
	if last_hit_e and Orbwalker.CurrentMode != Orbwalker.ModeKite:
		ctx.pill('LastHitE', Col.Green, Col.Black)
		
		player = ctx.player
		targets = ctx.minions.enemy_to(player).targetable().near(player, 700).get()
		
		e_spell = player.spells[Slot.E]
		e_dmg = calculate_raw_spell_dmg(player, e_spell)
		
		for target in targets:
			if target.health < e_dmg.calc_against(ctx, player, target):
				ctx.cast_spell(e_spell, target.pos)
				return
	