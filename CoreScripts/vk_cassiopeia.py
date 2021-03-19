from valkyrie import *			
from time import time
from helpers.targeting import TargetSelector, TargetSet
from helpers.spells import SpellRotation, RSpell, Slot
from helpers.templates import ChampionScript
import json


cassiopeia = ChampionScript(
	target_selector = TargetSelector(0, TargetSet.Champion),
	combat_key      = 0,
	harras_on       = False
)
last_hit_e = True

def calc_e_dmg(player):
	dmg = 48 + 4*player.lvl
	dmg += 0.1*player.ap
	
	return dmg
	
def condition_is_poisoned(ctx, player, target, spell):
	return target.has_buff('cassiopeiaqdebuff') or target.has_buff('cassiopeiawbuff')

def valkyrie_menu(ctx) :		 
	global cassiopeia, last_hit_e
	ui = ctx.ui					 
	
	cassiopeia.ui(ctx)
	last_hit_e = ui.checkbox('Auto last hit with E (No prediction)', last_hit_e)
	
def valkyrie_on_load(ctx) :	 
	global cassiopeia, last_hit_e
	cfg = ctx.cfg				 
	
	cassiopeia = ChampionScript.from_str(cfg.get_str('cassiopeia', str(cassiopeia)))
	cassiopeia.setup_combat(850, SpellRotation([RSpell(Slot.W), RSpell(Slot.Q), RSpell(Slot.E)]))
	cassiopeia.setup_harras(850, SpellRotation([RSpell(Slot.Q), RSpell(Slot.E, condition_is_poisoned)]))
	
	last_hit_e = cfg.get_bool('last_hit_e', False)
	
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	
	cfg.set_bool('last_hit_e', last_hit_e)
	cfg.set_str('cassiopeia', str(cassiopeia))
	
def valkyrie_exec(ctx) :	     
	
	if ctx.player.dead:
		return
		
	cassiopeia.exec(ctx)
	if last_hit_e and not cassiopeia.in_combat(ctx):
		ctx.pill('LastHitE', Col.Green, Col.Black)
		
		player = ctx.player
		targets = ctx.minions.enemy_to(player).targetable().near(player, 700).get()
		e_dmg = calc_e_dmg(player)
		
		for target in targets:
			if target.health < e_dmg:
				ctx.cast_spell(player.spells[Slot.E], target.pos)
	
	