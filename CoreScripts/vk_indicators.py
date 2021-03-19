from valkyrie import *
from helpers.drawings import Circle
from helpers.prediction import *
from helpers.items import *
from helpers.damages import calculate_raw_spell_dmg
from time import time
import json

player_circle	     = Circle(0.0, 30, 1.0, Col.Green, False, True)
turret_circle_enemy  = Circle(0.0, 50, 1.0, Col.Red,   False, True)
turret_circle_ally   = Circle(0.0, 50, 1.0, Col.Blue, False, True)
show_minion_hit      = True
show_casting_spells  = True
show_missiles        = True

show_potential_dmg   = True
potential_dmg_mask   = [True, True, True, True, True, True]

def degree_to_rad(degrees):
	return 0.01745*degrees

_90_DEG_IN_RAD = degree_to_rad(90)

def valkyrie_menu(ctx):
	global player_circle, show_minion_hit
	global show_casting_spells, show_missiles, show_potential_dmg, potential_dmg_mask
	ui = ctx.ui
	
	ui.text('Circles', Col.Purple)
	player_circle.ui("Attack range circle settings", ctx)
	turret_circle_enemy.ui("Enemy turret range circle settings", ctx)
	turret_circle_ally.ui("Ally turret range circle settings", ctx)
	ui.separator()
	
	ui.text('Others', Col.Purple)
	show_minion_hit     = ui.checkbox("Show minion hit damage indicator", show_minion_hit)
	show_casting_spells = ui.checkbox("Draw nearby skillshots (being cast)", show_casting_spells)
	show_missiles       = ui.checkbox("Draw nearby skillshot missiles", show_missiles)
	ui.separator()
	
	ui.text("Potential Damage", Col.Purple)
	show_potential_dmg  = ui.checkbox("Draw potential damage indicator", show_potential_dmg)
	potential_dmg_mask[0] = ui.checkbox("Include Q", potential_dmg_mask[0]); ui.sameline()
	potential_dmg_mask[1] = ui.checkbox("Include W", potential_dmg_mask[1]); ui.sameline()
	potential_dmg_mask[2] = ui.checkbox("Include E", potential_dmg_mask[2]); ui.sameline()
	potential_dmg_mask[3] = ui.checkbox("Include R", potential_dmg_mask[3])
	ui.text('Currently few champs are supported, request a champ on discord if u really want it.')
	
def valkyrie_on_load(ctx):
	global player_circle, show_minion_hit, turret_circle_ally, turret_circle_enemy
	global show_casting_spells, show_missiles, show_potential_dmg, potential_dmg_mask
	cfg = ctx.cfg
	
	player_circle         = Circle.from_str(cfg.get_str("player_circle", str(player_circle)))	  
	turret_circle_enemy   = Circle.from_str(cfg.get_str("turret_circle_enemy", str(turret_circle_enemy)))
	turret_circle_ally    = Circle.from_str(cfg.get_str("turret_circle_ally", str(turret_circle_ally)))	
	
	show_minion_hit       = cfg.get_bool("show_minion_hit", show_minion_hit)
	show_missiles         = cfg.get_bool("show_missiles", show_missiles)
	show_casting_spells   = cfg.get_bool("show_casting_spells", show_casting_spells)
	show_potential_dmg    = cfg.get_bool("show_potential_dmg", show_potential_dmg)
	potential_dmg_mask    = json.loads(cfg.get_str("potential_dmg_mask", json.dumps(potential_dmg_mask)))
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_str("turret_circle_enemy", str(turret_circle_enemy))
	cfg.set_str("turret_circle_ally", str(turret_circle_ally))
	cfg.set_str("player_circle", str(player_circle))
	cfg.set_bool("show_minion_hit", show_minion_hit)
	cfg.set_bool("show_missiles", show_missiles)
	cfg.set_bool("show_casting_spells", show_casting_spells)
	cfg.set_bool("show_potential_dmg", show_potential_dmg)
	cfg.set_str("potential_dmg_mask", json.dumps(potential_dmg_mask))

def draw_rect(ctx, start_pos, end_pos, radius, color):
	
	dir = Vec3(end_pos.x - start_pos.x, 0, end_pos.z - start_pos.z).normalize()
	
	left_dir = dir.rotate_y(_90_DEG_IN_RAD) * radius
	right_dir = dir.rotate_y(-_90_DEG_IN_RAD) * radius
	
	p1 = Vec3(start_pos.x + left_dir.x,  start_pos.y + left_dir.y,  start_pos.z + left_dir.z)
	p2 = Vec3(end_pos.x + left_dir.x,    end_pos.y + left_dir.y,    end_pos.z + left_dir.z)
	p3 = Vec3(end_pos.x + right_dir.x,   end_pos.y + right_dir.y,   end_pos.z + right_dir.z)
	p4 = Vec3(start_pos.x + right_dir.x, start_pos.y + right_dir.y, start_pos.z + right_dir.z)
	
	ctx.rect(p1, p2, p3, p4, 3, color)
	
def cast_draw_line(ctx, cast_info, static, collisions):
	start = cast_info.start_pos
	end = cast_info.end_pos
	if len(collisions) > 0:
		last_collision = collisions[-1]
		if last_collision.final:
			end = start + (cast_info.dir * last_collision.unit.pos.distance(start))
		
	draw_rect(ctx, start, end, static.width, Col.Gray)
	
def cast_draw_area(ctx, cast_info, static):
	
	fill_percent = min(1.0, (ctx.time - cast_info.time_begin)/cast_info.cast_time)
	ctx.circle(cast_info.end_pos, static.cast_radius, 30, 3.0, Col.Gray)
	ctx.circle_fill(cast_info.end_pos, static.cast_radius*fill_percent, 30, Col(0.5, 0.5, 0.5, 0.5))
	
def cast_draw_cone(ctx, cast_info, static):
	
	start = cast_info.start_pos
	
	angle = degree_to_rad(static.cast_cone_angle/2.0)
	direction = cast_info.dir
	
	left  = direction.rotate_y(-angle) * static.cast_cone_distance
	right = direction.rotate_y(angle) * static.cast_cone_distance
	ctx.triangle(start, left, right, 5.0, Col.Gray)
	
	fill_percent = min(1.0, (ctx.time - cast_info.time_begin)/cast_info.cast_time)
	left  = direction.rotate_y(-angle) * (static.cast_cone_distance*fill_percent)
	right = direction.rotate_y(angle) * (static.cast_cone_distance*fill_percent)
	
	ctx.triangle_fill(start, left + start, right + start, Col(0.5, 0.5, 0.5, 0.5))
	
def draw_collisions(ctx, collisions):
	for col in collisions:
		ctx.circle(col.unit.pos, col.unit.static.gameplay_radius, 30, 1.0, Col.Red)
		
		ctx.circle(Vec3(col.unit_pos.x, col.unit.pos.y, col.unit_pos.y), col.unit.static.gameplay_radius, 20, 3, Col.Blue)
		#ctx.circle(Vec3(col.spell_pos.x, col.unit.pos.y, col.spell_pos.y), col.spell.static.width, 20, 3, Col.Purple)

def draw_missile(ctx, missile):
	static = missile.spell.static
	if static == None:
		return
	
	cast_info = missile.spell
	start = missile.pos.clone()
	start.y = cast_info.start_pos.y
	end = cast_info.end_pos
	
	collisions = ctx.collisions_for(missile.spell)
	#draw_collisions(ctx, collisions)
		
	if static.has_flag(Spell.TypeLine):
		
		if len(collisions) > 0:
			last_collision = collisions[-1]
			if last_collision.final:
				end = start + (cast_info.dir * last_collision.unit.pos.distance(start))
				
		draw_rect(ctx, start, end, static.width, Col.Yellow)
	elif static.has_flag(Spell.TypeArea):
		
		fill_percent = min(1.0, 1.0 - start.distance(cast_info.end_pos)/cast_info.start_pos.distance(end))
		ctx.circle(cast_info.end_pos, static.cast_radius, 30, 3.0, Col.Yellow)
		ctx.circle_fill(cast_info.end_pos, static.cast_radius*fill_percent, 30, Col(1, 1, 0, 0.4))
	
def draw_cast(ctx, champ):
	cast_info = champ.curr_casting
	if not cast_info or not cast_info.static:
		return
		
	static = cast_info.static
	collisions = ctx.collisions_for(cast_info)
	#draw_collisions(ctx, collisions)
		
	if static.has_flag(Spell.TypeLine):
		cast_draw_line(ctx, cast_info, static, collisions)
	elif static.has_flag(Spell.TypeArea):
		cast_draw_area(ctx, cast_info, static)
	elif static.has_flag(Spell.TypeCone):
		cast_draw_cone(ctx, cast_info, static)

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
	
def draw_potential_dmg(ctx):
	
	dmgs = []
	player = ctx.player
	for i in range(4):
		if not potential_dmg_mask[i]:
			continue
			
		spell = player.spells[i]
		if spell.cd == 0.0 and spell.lvl > 0:
			dmgs.append(calculate_raw_spell_dmg(player, spell))
	
	for target in ctx.champs.enemy_to(player).targetable().on_screen().get():
		total_dmg = 0.0
		for dmg in dmgs:
			total_dmg += dmg.calc_against(player, target)
		ctx.hp_dmg_indicator(target, total_dmg, Col(1.0, 0.5, 0.1, 0.5))
		
	
def valkyrie_exec(ctx):
	draw_player_range(ctx)
	draw_turret_range(ctx)
	
	if show_minion_hit:
		draw_minion_hit_indicators(ctx)
	
	if show_casting_spells:
		for champ in ctx.champs.casting().enemy_to(ctx.player).near(ctx.player, 3000).get():
			draw_cast(ctx, champ)
	
	if show_missiles:
		for missile in ctx.missiles.enemy_to(ctx.player).near(ctx.player, 3000).get():
			draw_missile(ctx, missile)
			
	if show_potential_dmg:
		draw_potential_dmg(ctx)