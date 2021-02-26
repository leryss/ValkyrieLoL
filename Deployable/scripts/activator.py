from valkyrie          import *
from helpers.targeting import *
from helpers.inputs    import KeyInput
from helpers.spells    import Buffs, BuffType, CCType
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

class ActivatorQSS:
	
	all_cleanses         = { 'summonerboost', 'quicksilversash', '6035_spell', 'itemmercurial' }
	cleanses_suppress    = { 'quicksilversash', '6035_spell', 'itemmercurial' }
	
	default_enable = { str(type): True for type in CCType.Names.keys() }
	
	def __init__(self, delay = 0.1, min_cc_duration = 1.0, buff_settings = default_enable):
		self.delay = delay
		self.min_cc_duration = min_cc_duration
		self.buff_settings = buff_settings
		
	def ui(self, ui):
		
		for buff, enabled in self.buff_settings.items():
			self.buff_settings[buff] = ui.checkbox(f'Cleanse on {CCType.Names[int(buff)]}', enabled)
			
		self.delay           = ui.sliderfloat('Cast delay (secs)', self.delay, 0.0, 1.0)
		self.min_cc_duration = ui.sliderfloat('Minimum CC duration (secs)', self.min_cc_duration, 0.0, 3.0)
		
	def check(self, ctx, spell):
		
		for buff in ctx.player.buffs:
			buff_info = Buffs.get(buff.name)
			if buff_info.is_type(BuffType.CC):
				
				if self.buff_settings[str(buff_info.type_info)]:
					if ctx.time - buff.time_begin < self.delay:
						return None
					if buff.time_end - buff.time_begin < self.min_cc_duration:
						return None
					
					return ctx.player
					
		return None
		
	def get_icon(self):
		return 'summoner_boost'
		
	def get_name(self):
		return 'QSS/Cleanse'
		
	def __str__(self):
		return json.dumps([self.delay, self.min_cc_duration, self.buff_settings])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorQSS(delay = j[0], min_cc_duration = j[1], buff_settings = j[2])
		
class ActivatorSmite:
	
	smite_dmg       = [390, 410, 430, 450, 480, 510, 540, 570, 600, 640, 680, 720, 760, 800, 850, 900, 950, 1000]
	
	def __init__(self, key = KeyInput(0, False), selector_monster = TargetSelector(0, TargetSet.Monster)):
		self.key = key
		self.sel_monster = selector_monster
		
	def ui(self, ui):
		self.key.ui('Activation key', ui)
		self.sel_monster.ui('Targeting Monsters', ui)
		
	def check(self, ctx, spell):
		if self.key.check(ctx):
			ctx.pill('AutoSmite', Col.Black, Col.Yellow)
			
			target = self.sel_monster.get_target(ctx, ctx.jungle, 500.0)
			if target and target.health - self.smite_dmg[ctx.player.lvl - 1] <= 0.0 and (target.has_tags(Unit.MonsterLarge) or target.has_tags(Unit.MonsterEpic)):
				return target
			
		return None
		
	def get_icon(self):
		return 'summoner_smite'
		
	def get_name(self):
		return 'Smite'
		
	def __str__(self):
		return json.dumps([str(self.key), str(self.sel_monster)])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorSmite(KeyInput.from_str(j[0]), TargetSelector.from_str(j[1]))
		

activators = {
	'Smite': ActivatorSmite(),
	'QSS'  : ActivatorQSS()
}

spell_to_activator = {
	'summonersmite'                : 'Smite',
	's5_summonersmiteduel'         : 'Smite',
	's5_summonersmiteplayerganker' : 'Smite',
	
	'summonerboost'                : 'QSS',
	'quicksilversash'              : 'QSS', # Quicksilver active
	'6035_spell'                   : 'QSS', # Silvermere Dawn active
	'itemmercurial'                : 'QSS'  # Mercurial active
}

def valkyrie_menu(ctx):
	global activators
	ui = ctx.ui
	
	for name, activator in activators.items():
		ui.image(activator.get_icon(), Vec2(16, 16))
		ui.sameline()
		if ui.beginmenu(activator.get_name()):
			activator.ui(ui)
			ui.endmenu()

def valkyrie_on_load(ctx):
	global Smite
	cfg = ctx.cfg
	
	for name, activator in activators.items():
		activators[name] = eval(f'Activator{name}.from_str(cfg.get_str(name, str(activator)))')

def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	for name, activator in activators.items():
		cfg.set_str(name, str(activator))

cast_timestamps = [0.0] * 12 # Used to make sure we dont cast a spell 100 times a second

def valkyrie_exec(ctx):
	global cast_timestamps
	player = ctx.player
	
	for i, spell in enumerate(player.spells):
		if spell.lvl == 0 or spell.cd > 0.0:
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
	
	