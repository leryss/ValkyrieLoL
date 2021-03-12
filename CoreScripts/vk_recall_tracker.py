from valkyrie import *
from time import time
from vk_gank_awareness import set_champ_last_position

recalls = {}
bar_width   = 300
bar_height  = 30
show_mock = False 

flags_draggable     = WindowFlag.NoBackground | WindowFlag.AlwaysAutoResize | WindowFlag.NoTitleBar
flags_not_draggable = flags_draggable | WindowFlag.NoMove

team_base_pos = {
	100: Vec3(0, 0, 0),
	200: Vec3(14500, 0, 14500)
}

def valkyrie_menu(ctx):
	global bar_width, bar_height, show_mock
	ui = ctx.ui
	
	ui.text('Settings', Col.Purple)
	bar_width = ui.sliderfloat("Bar width", bar_width, 100, 500)
	bar_height = ui.sliderfloat("Bar height", bar_height, 10, 50)
	
	show_mock = ui.checkbox("Show test (drag the bars to change position)", show_mock)
	
def valkyrie_on_load(ctx):
	global bar_width, bar_height
	cfg = ctx.cfg
	
	bar_width = cfg.get_float("bar_width", bar_width)
	bar_height = cfg.get_float("bar_height", bar_height)
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_float("bar_width", bar_width)
	cfg.set_float("bar_height", bar_height)

def draw_bar(ui, champ_name, tleft):
	ui.image(champ_name + '_square', Vec2(bar_height, bar_height))
	ui.sameline()
	ui.progressbar(tleft/8.0, Vec2(bar_width, bar_height), f"{tleft:.2f}")

def valkyrie_exec(ctx):
	global recalls
	
	ui = ctx.ui
	ui.begin("Recalls", flags_draggable if show_mock else flags_not_draggable)
	
	if not show_mock:
		for champ in ctx.champs.enemy_to(ctx.player).get():
				
			timestamp, was_recalling = recalls.get(champ.net_id, (0, False))
			tleft = 8.0 - (time() - timestamp)
			if champ.recalling:
				if not was_recalling:
					was_recalling = True
					timestamp = time()
				
				draw_bar(ui, champ.name, tleft)
				
			elif was_recalling:
				if tleft < 0.05:
					set_champ_last_position(champ, team_base_pos[champ.team])
				was_recalling = False
				
			recalls[champ.net_id] = (timestamp, was_recalling)
	else:
		draw_bar(ui, 'garen', 3.5)
		draw_bar(ui, 'yone', 7.8)
		draw_bar(ui, 'anivia', 5.2)
		draw_bar(ui, 'annie', 2.1)
		draw_bar(ui, 'kennen', 1.1)
			
	
	ui.end()
		