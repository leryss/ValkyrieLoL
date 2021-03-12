from valkyrie import *
from helpers.drawings import draw_spell_track
import json

show_enemies = None
show_allies  = None
show_self	= None

settings = [
	["Q", [35, 10],   10, 25, True], 
	["W", [60, 10],   10, 25, True], 
	["E", [85, 10],   10, 25, True], 
	["R", [109, 10],  10, 25, True], 
	["D", [149, -15], 10, 25, True], 
	["F", [176, -15], 10, 25, True]
]
	
def valkyrie_menu(ctx):
	global show_enemies, show_allies, show_self, settings
	
	ui = ctx.ui
	ui.text('Show settings', Col.Purple)
	show_enemies = ui.checkbox("Show for enemies", show_enemies)
	show_allies  = ui.checkbox("Show for allies", show_allies)
	show_self	= ui.checkbox("Show for self", show_self)
	
	ui.separator()
	ui.text('Customization', Col.Purple)
	for i, (name, offset, rounding, size, show) in enumerate(settings):
		
		spell = ctx.player.spells[i]
		ui.image(spell.static.icon if spell.static else 'none', Vec2(16, 16), Col.White)
		ui.sameline()
		if ui.beginmenu(f'Customize {name}'):
			settings[i][4] = ui.checkbox("Show", show)
			settings[i][2] = ui.sliderint("Rounding", int(rounding), 0, 15)
			settings[i][3] = ui.sliderint("Size", int(size), 8, 30)
			offset[0] = ui.sliderint("X coord", int(offset[0]), -50, 200)
			offset[1] = ui.sliderint("Y coord", int(offset[1]), -60, 50)
			ui.endmenu()
	

def valkyrie_on_load(ctx):
	global show_enemies, show_allies, show_self, settings
	cfg = ctx.cfg
	
	show_enemies = cfg.get_bool("show_enemies", True)
	show_allies  = cfg.get_bool("show_allies", True)
	show_self	= cfg.get_bool("show_self", True)
	
	settings = json.loads(cfg.get_str("settings", json.dumps(settings)))
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_bool("show_enemies", show_enemies)
	cfg.set_bool("show_allies",  show_allies)
	cfg.set_bool("show_self",	show_self)
	
	cfg.set_str("settings", json.dumps(settings))

def draw_tracker_for(ctx, champ):
	pos = champ.hpbar_pos
	spells = champ.spells
	for i in range(0, 6):
		name, offset, rounding, size, show = settings[i]
		if show:
			draw_spell_track(ctx, spells[i], Vec2(pos.x + offset[0], pos.y + offset[1]), size, rounding)
	
def valkyrie_exec(ctx):
	
	player = ctx.player
	if show_self and ctx.is_on_screen(player.pos):
		draw_tracker_for(ctx, player)
				
	if show_enemies:
		for champ in ctx.champs.alive().visible().enemy_to(player).not_clone().on_screen().get():
			draw_tracker_for(ctx, champ)
			
	if show_allies:
		for champ in ctx.champs.alive().visible().ally_to(player).not_clone().on_screen().get():
			if champ != player:
				draw_tracker_for(ctx, champ)