from valkyrie import *
from helpers.drawings import Circle
import json

script_info = {
    'author': 'leryss',
    'description': 'none',
    'name': 'Vision Tracker'
}

show_clones, show_wards, show_traps = None, None, None

traps = {
	#Name -> (radius, show_radius_circle, show_radius_circle_minimap, icon)                      
	'caitlyntrap'          : [50,  True,  False, "caitlyn_yordlesnaptrap"],
	'jhintrap'             : [140, True,  False, "jhin_e"],
	'jinxmine'             : [50,  True,  False, "jinx_e"],
	'maokaisproutling'     : [50,  False, False, "maokai_e"],
	'nidaleespear'         : [50,  True,  False, "nidalee_w1"],
	'shacobox'             : [300, True,  False, "jester_deathward"],
	'teemomushroom'        : [75,  True,  True,  "teemo_r"]
}

wards = {
	'jammerdevice'         : [900, True, True, "pinkward"],
	'perkszombieward'      : [900, True, True, "bluetrinket"],
	'bluetrinket'          : [900, True, True, "bluetrinket"],
	'yellowtrinket'        : [900, True, True, "yellowtrinket"],
	'yellowtrinketupgrade' : [900, True, True, "yellowtrinket"],
    'sightward'            : [900, True, True, "sightward"],
	'visionward'           : [900, True, True, "sightward"],
	'ward'                 : [900, True, True, "sightward"],
}

clones = {
	'shaco'                : [0, False, False, "shaco_square"],
	'leblanc'              : [0, False, False, "leblanc_square"],
	'monkeyking'           : [0, False, False, "monkeyking_square"],
	'neeko'                : [0, False, False, "neeko_square"],
	'fiddlesticks'         : [0, False, False, "fiddlesticks_square"],
}

def draw_settings(ui, objs, label):
    ui.text(label)
    for x in objs.keys():
        ui.image(objs[x][-1], Vec2(15, 15))
        ui.sameline()
        if ui.treenode(x):
            objs[x][1] = ui.checkbox("Show range circles", objs[x][1])
            objs[x][2] = ui.checkbox("Show range circles on minimap", objs[x][2])
            
            ui.treepop()

def valkyrie_menu(ctx):
    global show_clones, show_wards, show_traps, traps, wards
    ui = ctx.ui
    
    show_clones = ui.checkbox("Show clones", show_clones)
    show_wards  = ui.checkbox("Show wards", show_wards)
    show_traps  = ui.checkbox("Show traps", show_traps)

    draw_settings(ui, traps, "Traps")
    draw_settings(ui, wards, "Wards")

def valkyrie_on_load(ctx):
    global show_clones, show_wards, show_traps, traps, wards
    cfg = ctx.cfg
    
    show_clones = cfg.get_bool("show_clones", True)
    show_wards = cfg.get_bool("show_wards", True)
    show_traps = cfg.get_bool("show_traps", True)
    
    traps = json.loads(cfg.get_str("traps", json.dumps(traps)))
    wards = json.loads(cfg.get_str("wards", json.dumps(wards)))
    
def valkyrie_on_save(ctx):
    cfg = ctx.cfg
    
    cfg.set_bool("show_clones", show_clones)
    cfg.set_bool("show_wards", show_wards)
    cfg.set_bool("show_traps", show_traps)
	
    cfg.set_str("traps", json.dumps(traps))
    cfg.set_str("wards", json.dumps(wards))
    
def draw(ctx, obj, radius, show_circle_world, show_circle_map, icon):
	
	
    pos = ctx.w2s(obj.pos)
    if ctx.is_on_screen(pos):
        duration = obj.expiry + obj.last_seen - ctx.time
        ctx.image(icon, pos, Vec2(30, 30), Col.White, 10)
        if duration > 0.0:
            pos.y += 25
            ctx.text(pos, str(int(duration)), Col.White)
        if show_circle_world:
            ctx.circle(obj.pos, radius, 50, 2, Col.Red)
	
    if show_circle_map:
        ctx.circle(ctx.w2m(obj.pos), ctx.d2m(radius), 15, 2, Col.Red)

def valkyrie_exec(ctx):
    
    for obj in ctx.others:
        if obj.ally_to(ctx.player) or obj.dead:
            continue
            
        if show_wards and obj.has_tags(Unit.Ward) and obj.name in wards:
            draw(ctx, obj, *(wards[obj.name]))
        elif show_traps and obj.has_tags(Unit.SpecialTrap) and obj.name in traps:
            draw(ctx, obj, *(traps[obj.name]))
	
    if show_clones:
        for champ in ctx.champs:
            if champ.ally_to(ctx.player) or champ.dead:
                continue
                
            # Clones have the same name for summoner spells
            if champ.name in clones and champ.spells[4].name == champ.spells[5].name:
                draw(ctx, champ, *(clones[champ.name]))