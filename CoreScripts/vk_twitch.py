from valkyrie import *
from helpers.spells import Slot
from helpers.damages import calculate_raw_spell_dmg

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
	if spell.lvl == 0 or spell.cd > 0.0 or spell.mana > player.mana:
		return
	
	raw_dmg = calculate_raw_spell_dmg(player, spell)
	for champ in ctx.champs.enemy_to(player).targetable().near(player, 1200.0).get():
		if champ.health - raw_dmg.calc_against(player, champ) <= 0.0:
			ctx.cast_spell(spell, None)
			break