from valkyrie import *			 
from helpers.drawings import Line, Image

line_world = Line(Col.Green, 1.0, True)
line_map   = Line(Col.Yellow, 1.0, True)
img_champ  = Image(Col.White, 10.0, 30.0, True)
min_path_len = 500

def valkyrie_menu(ctx) :		 
	global min_path_len
	ui = ctx.ui
	
	line_world.ui('Line path world', ctx)
	line_map.ui('Line path minimap', ctx)
	img_champ.ui('Image champion', ctx)
	min_path_len = ui.sliderint('Minimum path length for drawing', min_path_len, 0, 2000)
	
def valkyrie_on_load(ctx) :	 
	global line_world, line_map, img_champ
	global min_path_len
	cfg = ctx.cfg				 
	
	line_world = Line.from_str(cfg.get_str('line_world', str(line_world)))
	line_map   = Line.from_str(cfg.get_str('line_map', str(line_map)))
	img_champ  = Image.from_str(cfg.get_str('img_champ', str(img_champ)))
	min_path_len = cfg.get_int('min_path_len', min_path_len)
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	
	cfg.set_str('line_world', str(line_world))
	cfg.set_str('line_map', str(line_map))
	cfg.set_str('img_champ', str(img_champ))
	cfg.set_int('min_path_len', min_path_len)
	
def draw_champ_path(ctx, champ):
	path = champ.path
	
	len_path = len(path)
	if len_path > 1:
		dist = champ.path_distance()
		if dist < min_path_len:
			return
			
		for i in range(0, len_path - 1):
			line_world.draw_at(ctx, path[i], path[i + 1])
			if line_map.enabled:
				line_map.draw_at(ctx, ctx.w2m(path[i]), ctx.w2m(path[i + 1]))
			
		end_pos = ctx.w2s(path[len_path - 1])
		img_champ.draw_at(ctx, champ.name + '_square', end_pos)
		
		end_time = dist / (1.0 + (champ.dash_speed if champ.dashing else champ.move_speed))
		end_pos.y += img_champ.size/2.0 + 8.0
		ctx.text(end_pos, f'{end_time:.2f}', Col.White)
				
def valkyrie_exec(ctx) :	     
	
	for champ in ctx.champs.alive().enemy_to(ctx.player).get():
		draw_champ_path(ctx, champ)