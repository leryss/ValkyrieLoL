from valkyrie          import *
from helpers.targeting import *
from helpers.inputs    import KeyInput
import time
import json

script_info = {
	'author': 'leryss',
	'description': 'none',
	'name': 'Activator',
	'icon': 'menu-dev'
}

SummonerSpells = [
	"s5_summonersmiteduel", 
	"s5_summonersmiteplayerganker",
	"summonersmite",
	"summonerdot", 
	"summonerboost", 
	"summonerteleport",
	"summonerflash",
	"summonersnowball",
	"summonermana", 
	"summonerexhaust",
	"summonerbarrier",
	"summonerheal", 
	"summonerhaste"
]


class ActivatorSmite:
	
	smite_dmg       = [390, 410, 430, 450, 480, 510, 540, 570, 600, 640, 680, 720, 760, 800, 850, 900, 950, 1000]
	
	def __init__(self, icon, key, selector_monster):
		self.icon = icon
		self.key = key
		self.sel_monster = selector_monster
		
	def ui(self, ui):
		
		ui.image(self.icon, Vec2(16, 16))
		ui.sameline()
		if ui.beginmenu('Auto Smite'):
			self.key.ui('Activation key', ui)
			self.sel_monster.ui('Targeting Monsters', ui)
			ui.endmenu()
		
	def check(self, ctx, spell):
		if self.key.check(ctx):
			ctx.pill('AutoSmite', Col.Black, Col.Yellow)
			
			target = self.sel_monster.get_target(ctx, ctx.jungle, 500.0)
			if target and target.health - self.smite_dmg[ctx.player.lvl - 1] <= 0.0 and (target.has_tags(Unit.MonsterLarge) or target.has_tags(Unit.MonsterEpic)):
				return target
			
		return None
		
	def __str__(self):
		return json.dumps([self.icon, str(self.key), str(self.sel_monster)])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorSmite(j[0], KeyInput.from_str(j[1]), TargetSelector.from_str(j[2]))
		

activators = {
	'Smite': ActivatorSmite('summoner_smite', KeyInput(0, False), TargetSelector(0, TargetSet.Monster))
}

spell_to_activator = {
	'summonersmite'                : 'Smite',
	's5_summonersmiteduel'         : 'Smite',
	's5_summonersmiteplayerganker' : 'Smite'
}

def valkyrie_menu(ctx):
	global activators
	ui = ctx.ui
	
	for activator in activators.values():
		activator.ui(ui)

def valkyrie_on_load(ctx):
	global Smite
	cfg = ctx.cfg
	
	for name, activator in activators.items():
		activators[name] = eval(f'Activator{name}.from_str(cfg.get_str(name, str(activator)))')

def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	for name, activator in activators.items():
		cfg.set_str(name, str(activator))

cast_timestamps = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0] # Used to make sure we dont cast a spell 100 times a second

def valkyrie_exec(ctx):
	global cast_timestamps
	player = ctx.player
	
	for i, spell in enumerate(player.spells):
		if spell.cd > 0.0:
			continue
			
		now = time.time()
		if now - cast_timestamps[i] < 0.1:
			continue
			
		activator_name = spell_to_activator.get(spell.name, None)
		if activator_name:
			activator = activators[activator_name]
			target    = activator.check(ctx, spell)
			if target:
				ctx.cast_spell(spell, target.pos)
				cast_timestamps[i] = now
				break
	
	