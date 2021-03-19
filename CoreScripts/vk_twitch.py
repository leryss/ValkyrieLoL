from valkyrie import *
from helpers.spells import Slot

auto_e_killable = True

def valkyrie_menu(ctx) :
	global auto_e_killable, draw_e_indicator
	ui = ctx.ui	
	
	auto_e_killable = ui.checkbox('Auto E when killable', auto_e_killable)
	
def valkyrie_on_load(ctx) :	 
	global auto_e_killable, draw_e_indicator
	cfg = ctx.cfg				 
	
	auto_e_killable = cfg.get_bool('auto_e_killable', auto_e_killable)
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	
	cfg.set_bool('auto_e_killable', auto_e_killable)
	
def valkyrie_exec(ctx) :	     
	
	player = ctx.player
	if not auto_e_killable or player.dead:
		return

	spell = player.spells[Slot.E]
	if spell.cd > 0.0:
		return
	
	for champ in ctx.champs.enemy_to(player).targetable().near(player, 1200.0).get():
		dmg = calc_venom_dmg(player, champ, spell)
		
		if champ.health - dmg <= 0.0:
			ctx.cast_spell(spell, None)
			break