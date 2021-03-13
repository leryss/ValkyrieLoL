from valkyrie          import *
from helpers.targeting import *
from helpers.inputs    import KeyInput
from helpers.spells    import Buffs, BuffType, CCType
import time
import json

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

rechargable_actives = {
	2033, # Corruption pot
	2031  # Reffilable pot
}

class Enabler:

	enable_type = ['Always On', 'Key Input']

	def __init__(self, always_on, keyinput):
		self.keyinput  = keyinput
		self.selected  = 0 if always_on else 1
		
	def ui(self, ui):
		self.selected = ui.sliderenum('Activation Type', self.enable_type[self.selected], self.selected, 1)
		if self.selected == 1:
			self.keyinput.ui('Activation key', ui)
		ui.separator()

	def check(self, ctx):
		if self.selected == 0:
			return True
			
		return self.keyinput.check(ctx)
		
	def __str__(self):
		return json.dumps([self.selected == 0, str(self.keyinput)])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return Enabler(j[0], KeyInput.from_str(j[1]))


class ActivatorQSS:
	
	all_cleanses         = { 'summonerboost', 'quicksilversash', '6035_spell', 'itemmercurial' }
	cleanses_suppress    = { 'quicksilversash', '6035_spell', 'itemmercurial' }
	
	default_enable = { str(type): True for type in CCType.Names.keys() }
	
	def __init__(self, enabler = Enabler(True, KeyInput(0, False)), delay = 0.1, min_cc_duration = 1.0, min_slow_duration = 2.0, ms_breakpoint = 200.0, buff_settings = default_enable):
		self.enabler           = enabler 
		self.delay             = delay
		self.min_cc_duration   = min_cc_duration
		self.min_slow_duration = min_slow_duration
		self.ms_breakpoint     = ms_breakpoint
		self.buff_settings     = buff_settings
		
	def ui(self, ctx, ui):
		self.enabler.ui(ui)
		for buff, enabled in self.buff_settings.items():
			self.buff_settings[buff] = ui.checkbox(f'Cleanse on {CCType.Names[int(buff)]}', enabled)
		
		ui.separator()
		self.delay             = ui.sliderfloat('Cast delay (secs)', self.delay, 0.0, 1.0)
		self.min_cc_duration   = ui.sliderfloat('Minimum CC duration (secs)', self.min_cc_duration, 0.0, 3.0)
		self.min_slow_duration = ui.sliderfloat("Minimum slow duration (secs)", self.min_slow_duration, 0.0, 5.0)
		self.ms_breakpoint     = ui.sliderfloat('Movement speed less than (when slowed)', self.ms_breakpoint, 50.0, 300.0)
		
	def check(self, ctx, spell):
		
		if not self.enabler.check(ctx):
			return None
		
		#ctx.pill('QSS', Col.Black, Col.Cyan)
		for buff in ctx.player.buffs:
			buff_info = Buffs.get(buff.name)
			if buff_info.is_type(BuffType.CC):
				
				if self.buff_settings[str(buff_info.type_info)]:
					if ctx.time - buff.time_begin < self.delay:
						return None
						
					remaining_duration = buff.time_end - buff.time_begin
					if buff_info.type_info == CCType.Slow:
						if remaining_duration < self.min_slow_duration and ctx.player.move_speed > self.ms_breakpoint:
							return None
					else:
						if remaining_duration < self.min_cc_duration:
							return None
					
					return ctx.player
					
		return None
		
	def get_icon(self):
		return 'summoner_boost'
		
	def get_name(self):
		return 'QSS/Cleanse'
		
	def __str__(self):
		return json.dumps([str(self.enabler), self.delay, self.min_cc_duration, self.min_slow_duration, self.ms_breakpoint, self.buff_settings])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorQSS(enabler = Enabler.from_str(j[0]), delay = j[1], min_cc_duration = j[2], min_slow_duration = j[3], ms_breakpoint = j[4], buff_settings = j[5])
		
class ActivatorSmite:
	
	smite_dmg       = [390, 410, 430, 450, 480, 510, 540, 570, 600, 640, 680, 720, 760, 800, 850, 900, 950, 1000]
	
	def __init__(self, enabler = Enabler(False, KeyInput(0, False)), selector_monster = TargetSelector(0, TargetSet.Monster)):
		self.enabler = enabler
		self.sel_monster = selector_monster
		
	def ui(self, ctx, ui):
		self.enabler.ui(ui)
		self.sel_monster.ui('Targeting Monsters', ctx, ui)
		
	def check(self, ctx, spell):
		if self.enabler.check(ctx):
			ctx.pill('Smite', Col.Black, Col.Yellow)
			
			target = self.sel_monster.get_target(ctx, ctx.jungle, 500.0)
			if target and target.health - self.smite_dmg[ctx.player.lvl - 1] <= 0.0 and (target.has_tags(Unit.MonsterLarge) or target.has_tags(Unit.MonsterEpic)):
				return target
			
		return None
		
	def get_icon(self):
		return 'summoner_smite'
		
	def get_name(self):
		return 'Smite'
		
	def __str__(self):
		return json.dumps([str(self.enabler), str(self.sel_monster)])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorSmite(Enabler.from_str(j[0]), TargetSelector.from_str(j[1]))
		
class ActivatorPotion:

	pot_buffs = {
		'item2010'             : 'Item2010',
		'item2003'             : 'Item2003',
		'itemcrystalflask'     : 'ItemCrystalFlask',
		'itemdarkcrystalflask' : 'ItemDarkCrystalFlask'
	}

	def __init__(self, enabler = Enabler(True, KeyInput(0, False)), hp_breakpoint = 80.0):
		self.enabler = enabler
		self.hp_breakpoint = hp_breakpoint
		
	def ui(self, ctx, ui):
		self.enabler.ui(ui)
		self.hp_breakpoint = ui.sliderfloat('When below % HP', self.hp_breakpoint, 0.0, 99.0)
		
	def check(self, ctx, spell):
		if self.enabler.check(ctx):
			buff = self.pot_buffs.get(spell.name, None)
			if not buff:
				return None
			
			# Check if pot already in effect
			player = ctx.player
			if Buffs.has_buff(player, buff):
				return None
			
			#ctx.pill('Potion', Col.White, Col.Red)
			if player.health/player.max_health <= self.hp_breakpoint/100.0:
				return ctx.player
			
		return None
		
	def get_icon(self):
		return 'potion'
		
	def get_name(self):
		return 'Potion'
		
	def __str__(self):
		return json.dumps([str(self.enabler), self.hp_breakpoint])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return ActivatorPotion(enabler = Enabler.from_str(j[0]), hp_breakpoint = j[1])

activators = {
	'Smite': ActivatorSmite(),
	'QSS'  : ActivatorQSS(),
	'Potion' : ActivatorPotion()
}

spell_to_activator = {
	'summonersmite'                : 'Smite',
	's5_summonersmiteduel'         : 'Smite',
	's5_summonersmiteplayerganker' : 'Smite',
	
	'summonerboost'                : 'QSS',
	'quicksilversash'              : 'QSS', # Quicksilver active
	'6035_spell'                   : 'QSS', # Silvermere Dawn active
	'itemmercurial'                : 'QSS', # Mercurial active
	
	'item2010'                     : 'Potion', # Biscuit
	'item2003'                     : 'Potion', # Red potion
	'itemcrystalflask'             : 'Potion', # Refillable potion
	'itemdarkcrystalflask'         : 'Potion'  # Corruption potion
}

def valkyrie_menu(ctx):
	global activators
	ui = ctx.ui
	
	ui.text('Activables', Col.Purple)
	for name, activator in activators.items():
		ui.image(activator.get_icon(), Vec2(16, 16))
		ui.sameline()
		if ui.beginmenu(activator.get_name()):
			activator.ui(ctx, ui)
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

used_activators = set([])

def try_activate(ctx, player, spell, item_slot = None):
	global used_activators
	
	activator_name = spell_to_activator.get(spell.name, None)
	if activator_name and activator_name not in used_activators:
		used_activators.add(activator_name)
		activator = activators[activator_name]
		target    = activator.check(ctx, spell)
		if target and ctx.cast_spell(spell, target.pos):
			ctx.info(f"Casted spell {spell.name}")
			return True
			
	return False
	
def valkyrie_exec(ctx):
	global used_activators
	player = ctx.player
	
	if player.dead:
		return
	
	used_activators = set({})
	spells = player.spells
	if try_activate(ctx, player, spells[4]): #D slot
		return
	if try_activate(ctx, player, spells[5]): #F slot
		return
	
	for slot in player.item_slots:
		if not slot.active or ctx.is_at_spawn(player):
			continue
		if slot.item.id in rechargable_actives and slot.charges == 0:
			continue
			
		if try_activate(ctx, player, slot.active):
			return
		