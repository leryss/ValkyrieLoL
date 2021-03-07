from valkyrie import *
from helpers.drawings import Circle
from helpers.prediction import *
from helpers.items import *
from time import time

player_circle	     = Circle(0.0, 30, 1.0, Col.Green, False, True)
turret_circle_enemy  = Circle(0.0, 50, 1.0, Col.Red,   False, True)
turret_circle_ally   = Circle(0.0, 50, 1.0, Col.Blue, False, True)
show_minion_hit      = True

def valkyrie_menu(ctx):
	global player_circle, show_minion_hit
	ui = ctx.ui
	
	player_circle.ui("Attack range circle settings", ctx)
	turret_circle_enemy.ui("Enemy turret range circle settings", ctx)
	turret_circle_ally.ui("Ally turret range circle settings", ctx)
	show_minion_hit = ui.checkbox("Show minion hit damage indicator", show_minion_hit)
	
def valkyrie_on_load(ctx):
	global player_circle, show_minion_hit, turret_circle_ally, turret_circle_enemy
	cfg = ctx.cfg
	
	player_circle         = Circle.from_str(cfg.get_str("player_circle", str(player_circle)))	  
	turret_circle_enemy   = Circle.from_str(cfg.get_str("turret_circle_enemy", str(turret_circle_enemy)))
	turret_circle_ally    = Circle.from_str(cfg.get_str("turret_circle_ally", str(turret_circle_ally)))	
	
	show_minion_hit       = cfg.get_bool("show_minion_hit", show_minion_hit)
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_str("turret_circle_enemy", str(turret_circle_enemy))
	cfg.set_str("turret_circle_ally", str(turret_circle_ally))
	cfg.set_str("player_circle", str(player_circle))
	cfg.set_bool("show_minion_hit", show_minion_hit)

def draw_minion_hit_indicators(ctx):
	player  = ctx.player
	minions = ctx.minions.enemy_to(player).targetable().on_screen().get()
	
	
	for minion in minions:
		hit_dmg = get_onhit_physical(player, minion) + items.get_onhit_magical(player, minion)
		
		percent_curr = minion.health/minion.max_health
		percent_after_hit = (minion.health - hit_dmg)/minion.max_health
		percent_after_hit = percent_after_hit if percent_after_hit > 0.0 else 0.0
		
		hp_bar_pos = minion.hpbar_pos
		hp_bar_pos.y -= 5.5
		hp_bar_pos.x += percent_after_hit*62
		
		ctx.rect_fill(hp_bar_pos, Vec2((percent_curr - percent_after_hit)*62 + 1, 3.5), Col(1, 0.8, 0.5, 0.7) if minion.health - hit_dmg > 0.0 else Col(0.1, 1, 0.35, 0.7))
		ctx.line(hp_bar_pos + Vec2(0, -6), hp_bar_pos + Vec2(0, 10), 1, Col.Black)

def draw_turret_range(ctx):
	for turret in ctx.turrets.alive().on_screen().get():
		circle = turret_circle_ally if turret.ally_to(ctx.player) else turret_circle_enemy
		circle.radius = turret.static.base_atk_range + turret.static.gameplay_radius
		circle.draw_at(ctx, turret.pos)
		
def draw_player_range(ctx):
	player_circle.radius = ctx.player.atk_range + ctx.player.static.gameplay_radius
	player_circle.draw_at(ctx, ctx.player.pos)
	
def valkyrie_exec(ctx):
	draw_player_range(ctx)
	draw_turret_range(ctx)
	
	if show_minion_hit:
		draw_minion_hit_indicators(ctx)