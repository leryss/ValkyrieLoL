from valkyrie import *			 
from time import perf_counter

def valkyrie_menu(ctx) :		 
	ui = ctx.ui					 
	
def valkyrie_on_load(ctx) :	 
	cfg = ctx.cfg				 
	pass						 
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	pass						 
	
def degree_to_rad(degrees):
	return 0.01745*degrees
	
_90_DEG_IN_RAD = degree_to_rad(90)
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
		ctx.info(str(collisions))
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
		ctx.circle(col.unit.pos, 100, 30, 1.0, Col.Red)
		
		ctx.circle(Vec3(col.unit_pos.x, col.unit.pos.y, col.unit_pos.y), 55, 10, 5, Col.Blue)
		ctx.circle(Vec3(col.spell_pos.x, col.spell.end_pos.y, col.spell_pos.y), 55, 10, 5, Col.Purple)

def draw_missile(ctx, missile):
	static = missile.spell.static
	if static == None:
		return
	
	cast_info = missile.spell
	start = missile.pos.clone()
	start.y = cast_info.start_pos.y
	end = cast_info.end_pos
	
	collisions = ctx.collisions_for(missile.spell)
	draw_collisions(ctx, collisions)
		
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
	draw_collisions(ctx, collisions)
		
	if static.has_flag(Spell.TypeLine):
		cast_draw_line(ctx, cast_info, static, collisions)
	elif static.has_flag(Spell.TypeArea):
		cast_draw_area(ctx, cast_info, static)
	elif static.has_flag(Spell.TypeCone):
		cast_draw_cone(ctx, cast_info, static)
	

def draw_spells(ctx):
	
	for champ in ctx.champs.casting().near(ctx.player, 3000).get():
		draw_cast(ctx, ctx.player)
	
	for missile in ctx.missiles.near(ctx.player, 3000).get():
		draw_missile(ctx, missile)
		
	
def valkyrie_exec(ctx) :	     
	
	#m = [m for m in ctx.minions]
	#ctx.missiles.casting().get()
	draw_spells(ctx)