from valkyrie import *
from helpers.targeting import TargetSelector, TargetSet
from helpers.spells import SpellRotation, RSpell, Slot
from helpers.templates import ChampionScript

kogmaw = ChampionScript(
	harras_on       = False
)

ranges_ult = [1300, 1550, 1800]
max_r_stacks_harras = 1
minimum_q_defenses  = 0

def condition_e(ctx, player, target, spell):
	return player.pos.distance(target.pos) > 700

def condition_r_combat(ctx, player, target, spell):
	num_stacks = player.num_buff_stacks('kogmawlivingartillerycost')
	if num_stacks*spell.mana > player.mana:
		return False
		
	# Valkyrie engine doesnt support multiple ranges on the same skill so we have to handle it ourselves
	if player.pos.distance(target.pos) > ranges_ult[spell.lvl - 1]:
		return False
		
	return True
	
def condition_r_harras(ctx, player, target, spell):
	num_stacks = player.num_buff_stacks('kogmawlivingartillerycost')
	if num_stacks*spell.mana > player.mana:
		return False
		
	# Valkyrie engine doesnt support multiple ranges on the same skill so we have to handle it ourselves
	if player.pos.distance(target.pos) > ranges_ult[spell.lvl - 1]:
		return False
		
	if num_stacks > max_r_stacks_harras:
		return False
		
	return True

def condition_q(ctx, player, target, spell):
	if target.armor < minimum_q_defenses and target.magic_res < minimum_q_defenses:
		return False
		
	return True

def valkyrie_menu(ctx) :		 
	global max_r_stacks_harras, minimum_q_defenses
	ui = ctx.ui					 

	kogmaw.ui(ctx)
	max_r_stacks_harras = ui.sliderint('Max R stacks while harrasing', max_r_stacks_harras, 1, 9)
	minimum_q_defenses  = ui.sliderfloat('Minimum armor/magic resist for Q', minimum_q_defenses, 0.0, 200.0)
	
def valkyrie_on_load(ctx) :	 
	global kogmaw, max_r_stacks_harras, minimum_q_defenses
	cfg = ctx.cfg				 
	
	kogmaw = ChampionScript.from_str(cfg.get_str("kogmaw", str(kogmaw)))
	kogmaw.setup_combat(1800.0, SpellRotation([RSpell(Slot.Q, condition_q), RSpell(Slot.W), RSpell(Slot.E, condition_e), RSpell(Slot.R, condition_r_combat)]))
	kogmaw.setup_harras(1800.0, SpellRotation([RSpell(Slot.R, condition_r_harras)]))
	
	max_r_stacks_harras = cfg.get_int('max_r_stacks_harras', max_r_stacks_harras)
	minimum_q_defenses  = cfg.get_float('minimum_q_defenses', minimum_q_defenses)
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	cfg.set_str('kogmaw', str(kogmaw))
	cfg.set_int('max_r_stacks_harras', max_r_stacks_harras)
	cfg.set_float('minimum_q_defenses', minimum_q_defenses)
	
def valkyrie_exec(ctx) :	     
	if ctx.player.dead:
		return
	
	kogmaw.exec(ctx)
