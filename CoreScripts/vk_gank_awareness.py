from valkyrie import *
from time import time
from helpers.drawings import draw_spell_track

size_portrait_minimap = 24
size_portrait_world   = 48

max_gank_distance	= None
show_gank_alerts	 = None
show_last_seen_mm	= None
show_last_seen_world = None

def valkyrie_menu(ctx):
	global max_gank_distance, show_last_seen_mm, show_last_seen_world, show_gank_alerts
	global size_portrait_world
	ui = ctx.ui
	
	ui.text('Settings', Col.Purple)
	size_portrait_world   = ui.sliderfloat("Size champion portrait", size_portrait_world, 20, 80)
	max_gank_distance	 = ui.sliderfloat("Trigger distance", max_gank_distance, 1000, 10000)
	show_last_seen_mm	 = ui.checkbox("Show enemy last position on minimap", show_last_seen_mm)
	show_last_seen_world  = ui.checkbox("Show enemy last position on world", show_last_seen_world)
	show_gank_alerts	  = ui.checkbox("Show gank alerts", show_gank_alerts)
	
	
	ui.image("garen_square", Vec2(size_portrait_world, size_portrait_world))

def valkyrie_on_load(ctx):
	global max_gank_distance, show_last_seen_mm, show_last_seen_world, show_gank_alerts
	global size_portrait_world
	cfg = ctx.cfg
	
	max_gank_distance	= cfg.get_float("max_gank_distance", 4000)
	show_last_seen_mm	= cfg.get_bool("show_last_seen_mm", True)
	show_last_seen_world = cfg.get_bool("show_last_seen_world", True)
	show_gank_alerts	 = cfg.get_bool("show_gank_alerts", True)
	size_portrait_world  = cfg.get_float("size_portrait_world", size_portrait_world)
	
def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_float("max_gank_distance",   max_gank_distance)
	cfg.set_float("size_portrait_world", size_portrait_world)
	cfg.set_bool("show_last_seen_mm",	show_last_seen_mm)
	cfg.set_bool("show_last_seen_world", show_last_seen_world)
	cfg.set_bool("show_gank_alerts",	 show_gank_alerts)

def draw_portrait_world(ctx, champ, start, color = Col.Gray, distance = None):

	ctx.image(champ.name + '_square', start, Vec2(size_portrait_world, size_portrait_world), Col.White if champ.visible else color)
	draw_spell_track(ctx, champ.spells[4], start + Vec2(size_portrait_world/2, -20), 21, 15)
	draw_spell_track(ctx, champ.spells[5], start + Vec2(size_portrait_world/2, 3), 21, 15)
	
	if not champ.visible:
		ctx.text(start, f'{int(ctx.time - champ.last_seen)}', Col.Red)
		
	start.y += size_portrait_world/2.0
	ctx.rect_fill(start - Vec2(size_portrait_world/2.0, 0.0), Vec2(size_portrait_world*(champ.health/champ.max_health), 5.0), Col.Green)
	
	if distance != None:
		start.y += 15.0
		ctx.text(start, f'{distance:.0f}m', Col.White)

def draw_gank_alert(ctx, champ):
	player = ctx.player
	distance = champ.pos.distance(player.pos)
	if distance > max_gank_distance or ctx.is_on_screen(champ.pos):
		return
		
	pos = ctx.w2s(player.pos + (champ.pos - player.pos).normalize() * 500.0)
	draw_portrait_world(ctx, champ, pos, distance=distance)
	
last_positions = {}

def draw_champ_last_position(ctx, champ):
	global last_positions
	if champ.visible:
		return
		
	now = time()
	pos, timestamp_changed, override_pos = last_positions.get(champ.net_id, [None, None, None])
	color = Col.Gray
	if not timestamp_changed and pos and pos.x != champ.pos.x:
		override_pos = None
		timestamp_changed = now
	elif timestamp_changed:
		if now - timestamp_changed > 10.0:
			timestamp_changed = None
		else:
			color = Col.Green
	
	pos_world = ctx.w2s(override_pos if override_pos else champ.pos)
	pos_mm	  = ctx.w2m(override_pos if override_pos else champ.pos)
	
	if show_last_seen_mm:
		ctx.image(champ.name + '_square', pos_mm, Vec2(size_portrait_minimap, size_portrait_minimap), color, 10)
	
	if show_last_seen_world:
		draw_portrait_world(ctx, champ, pos_world, color=color)
		
	last_positions[champ.net_id] = [champ.pos, timestamp_changed, override_pos]
	
def set_champ_last_position(champ, new_pos):
	global last_positions
	
	last_positions[champ.net_id] = [champ.pos, time(), new_pos]
	
def valkyrie_exec(ctx):
	for champ in ctx.champs.alive().enemy_to(ctx.player).get():
		
		if show_gank_alerts:
			draw_gank_alert(ctx, champ)
		draw_champ_last_position(ctx, champ)
		
		