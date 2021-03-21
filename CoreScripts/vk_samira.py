from valkyrie import *
from helpers.targeting import TargetSelector, TargetSet
from helpers.spells import SpellRotation, RSpell, Slot
from helpers.templates import ChampionScript

samira = ChampionScript(
	harras_on       = False
)
leap_under_tower = False
leap_gap_closer  = True
leap_below_hp    = 30

# Cast W only when we can deflect missiles that is when skillshots will collide
def condition_w(ctx, player, target, spell):
	cols = ctx.collisions_for(player)
	for col in cols:
		if col.time_until_impact < 0.75:
			return True
			
	return False
	
# Use E as gap closer
def condition_e(ctx, player, target, spell):
	leap = False
	if leap_gap_closer and player.pos.distance(target.pos) > 400:
		leap = True
	elif target.health/target.max_health < float(leap_below_hp)/100.0:
		leap = True
	
	if leap:
		if ctx.is_under_tower(target) and not leap_under_tower:
			return False
		return True
	
	return False
	
def valkyrie_menu(ctx) :	
	global leap_under_tower, leap_gap_closer, leap_below_hp
	
	ui = ctx.ui					 
	samira.ui(ctx) 
	leap_under_tower = ui.checkbox('Use E under enemy tower', leap_under_tower)
	leap_gap_closer  = ui.checkbox('Use E as gap closer',     leap_gap_closer)
	leap_below_hp    = ui.sliderint('Leap on targets below % HP', leap_below_hp, 10, 99)
	
def valkyrie_on_load(ctx) :	 
	global samira
	global leap_under_tower, leap_gap_closer, leap_below_hp
	cfg = ctx.cfg				 
	
	samira = ChampionScript.from_str(cfg.get_str('samira', str(samira)))
	samira.setup_combat(650, SpellRotation([RSpell(Slot.R), RSpell(Slot.Q), RSpell(Slot.W, condition_w), RSpell(Slot.E, condition_e)]))
	samira.setup_harras(950, SpellRotation([RSpell(Slot.Q)]))
	
	leap_under_tower = cfg.get_bool('leap_under_tower', leap_under_tower)
	leap_gap_closer = cfg.get_bool('leap_gap_closer', leap_gap_closer)
	leap_below_hp = cfg.get_int('leap_below_hp', leap_below_hp)
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	cfg.set_str('samira', str(samira))
	cfg.set_bool('leap_under_tower', leap_under_tower)
	cfg.set_bool('leap_gap_closer', leap_gap_closer)
	cfg.set_int('leap_below_hp', leap_below_hp)
	
def valkyrie_exec(ctx) :
	if ctx.player.dead:
		return
		
	samira.exec(ctx)