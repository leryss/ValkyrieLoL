from valkyrie import *			 
from helpers.targeting import TargetSelector, TargetSet
import json

keys = [
	0, 0, 0, 0
]

channels = [
	False, False, False, False
]

target_sel = TargetSelector(0, TargetSet.Champion)
target_minions  = False
target_monsters = True

def valkyrie_menu(ctx):
	global target_minions, target_monsters
	ui = ctx.ui
	
	target_sel.ui('Target selector', ctx, ui)
	target_minions = ui.checkbox('Target minions', target_minions)
	target_monsters = ui.checkbox('Target jungle monsters', target_monsters)
	keys[0] = ui.keyselect('Key auto aim Q', keys[0])
	keys[1] = ui.keyselect('Key auto aim W', keys[1])
	keys[2] = ui.keyselect('Key auto aim E', keys[2])
	keys[3] = ui.keyselect('Key auto aim R', keys[3])
	ui.separator()
	ui.text("Don't bind keys to Q,W,E,R")
	ui.text("Currently doesnt support ally targeting")
	ui.text("Some champions arent tested yet")
	
def valkyrie_on_load(ctx) :	
	global keys, target_sel
	global target_minions, target_monsters
	cfg = ctx.cfg				 
	
	keys = json.loads(cfg.get_str('keys', str(keys)))
	target_sel = TargetSelector.from_str(cfg.get_str('target_sel', str(target_sel)))
	target_minions = cfg.get_bool('target_minions', target_minions)
	target_monsters = cfg.get_bool('target_monsters', target_monsters)
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	
	cfg.set_str('keys', str(keys))
	cfg.set_str('target_sel', str(target_sel))
	cfg.set_bool('target_minions', target_minions)
	cfg.set_bool('target_monsters', target_monsters)
	
def cast(ctx, spell, static, end_channel = False):
		
	if static.has_flag(Spell.DashSkill) or static.has_flag(Spell.CastAnywhere):
		ctx.cast_spell(spell, None)
		return
	
	player = ctx.player
	target = target_sel.get_target(ctx, ctx.champs.enemy_to(player).targetable().near(player, static.cast_range).get())
	if not target and target_minions:
		target = target_sel.get_target(ctx, ctx.minions.enemy_to(player).targetable().near(player, static.cast_range).get())
	if not target and target_monsters:
		target = target_sel.get_target(ctx, ctx.jungle.enemy_to(player).targetable().near(player, static.cast_range).get())
	
	if not target:
		if end_channel:
			ctx.cast_spell(spell, None)
		return
	
	point = ctx.predict_cast_point(player, target, spell)
	if point:
		ctx.cast_spell(spell, point)
	elif end_channel:
		ctx.cast_spell(spell, None)
	
def valkyrie_exec(ctx) :	     
	global channels
	
	spells = ctx.player.spells
	for i in range(4):
		spell = spells[i]
		static = spell.static
		if static == None:
			continue
		
		is_channel = static.has_flag(Spell.ChannelSkill)
		
		if is_channel and channels[i] and not ctx.is_held(keys[i]):
			cast(ctx, spell, static, True)
			channels[i] = False
		
		if ctx.was_pressed(keys[i]):
			if is_channel:
				channels[i] = ctx.start_channel(spell)
			else:
				cast(ctx, spell, static)
			
	
	
