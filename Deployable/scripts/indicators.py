from valkyrie import *
from helpers.drawings import Circle
from helpers.prediction import *
from helpers.items import *

script_info = {
	'author': 'leryss',
	'description': 'none',
	'name': 'Indicators',
	'icon': 'menu-pencil'
}

player_circle	= Circle(0.0, 30, 1.0, Col.Green, False, True)
show_minion_hit  = True

def valkyrie_menu(ctx):
	global player_circle, show_minion_hit, i
	ui = ctx.ui
	
	player_circle.ui("Attack range circle settings", ctx)
	show_minion_hit = ui.checkbox("Show minion hit damage indicator", show_minion_hit)
	
def valkyrie_on_load(ctx):
	global player_circle, show_minion_hit
	cfg = ctx.cfg
	
	player_circle   = Circle.from_str(cfg.get_str("player_circle", str(player_circle)))	  
	show_minion_hit = cfg.get_bool("show_minion_hit", show_minion_hit)
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_str("player_circle", str(player_circle))
	cfg.set_bool("show_minion_hit", show_minion_hit)
	
def draw_minion_hit_indicators(ctx):
	minions = [m for m in ctx.minions if ctx.is_on_screen(m.pos)]
	player  = ctx.player
	
	for minion in minions:
		if minion.dead or not minion.targetable or minion.ally_to(player) or not minion.visible:
			continue
			
		hit_dmg = get_onhit_physical(player, minion) + items.get_onhit_magical(player, minion)

		percent_curr = minion.health/minion.max_health
		percent_after_hit = (minion.health - hit_dmg)/minion.max_health
		percent_after_hit = percent_after_hit if percent_after_hit > 0.0 else 0.0
		
		hp_bar_pos = minion.hpbar_pos
		hp_bar_pos.y -= 5.5
		hp_bar_pos.x += percent_after_hit*62
		
		ctx.rect_fill(hp_bar_pos, Vec2((percent_curr - percent_after_hit)*62 + 1, 3.5), Col(1, 0.8, 0.5, 0.7) if minion.health - hit_dmg > 0.0 else Col(0.1, 1, 0.35, 0.7))
		ctx.line(hp_bar_pos + Vec2(0, -6), hp_bar_pos + Vec2(0, 10), 1, Col.Black)
	
def valkyrie_exec(ctx):
	
	player_circle.radius = ctx.player.atk_range + ctx.player.static.gameplay_radius
	player_circle.draw_at(ctx, ctx.player.pos)
	
	if show_minion_hit:
		draw_minion_hit_indicators(ctx)
	
	