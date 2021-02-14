from valkyrie          import *
from helpers.targeting import *
import time
import json

script_info = {
	'author': 'leryss',
	'description': 'none',
	'name': 'Activator',
	'icon': 'menu-dev'
}

class KeyToggleCondition:
	
	def __init__(self, key = 0):
		self.key = key
		self.toggled = False
		
	def ui(self, ctx):
		ctx.ui.text("Activates when toggled using the selected key")
		self.key = ctx.ui.keyselect("Toggle key", self.key)
		
	def check(self, ctx, target, spell):
		if ctx.was_pressed(self.key):
			self.toggled = ~self.toggled
		
		return self.toggled	
		
	def __str__(self):
		return json.dumps([self.key])
		
class KeyHoldCondition:
	
	def __init__(self, key = 0):
		self.key = key
		
	def ui(self, ctx):
		ctx.ui.text("Activates when holding down the selected key")
		self.key = ctx.ui.keyselect("Hold key", self.key)
		
	def check(self, ctx, target, spell):
		if ctx.is_held(self.key):
			return True
		return False	
		
	def __str__(self):
		return json.dumps([self.key])


class KillableCondition:
	
	smite_dmg = [390, 410, 430, 450, 480, 510, 540, 570, 600, 640, 680, 720, 760, 800, 850, 900, 950, 1000]
	
	def __init__(self):
		pass
		
	def ui(self, ctx):
		ctx.ui.text(f"Activates when target is killable with the activable spell")
		
	def check(self, ctx, target, spell):
		# Temporary added smite dmg hardcoded, but damage solvers should be implemented
		return target.health - (self.smite_dmg[ctx.player.lvl - 1] if 'smite' in spell.name else 0.0) <= 0.0
		
	def __str__(self):
		return '[]'

class SmitableCondition:
	
	def __init__(self):
		pass
		
	def ui(self, ctx):
		ctx.ui.text("Activates when target is smitable")
		
	def check(self, ctx, target, spell):
		return target.has_tags(Unit.MonsterEpic) or target.has_tags(Unit.MonsterLarge)
		
	def __str__(self):
		return '[]'
		
		
class HealthCondition:
	
	threshold = 30

	def __init__(self, threshold = 30):
		self.threshold = threshold
		
	def ui(self, ctx):
		ctx.ui.text("Activates when target is under a threshold % HP")
		self.threshold = ctx.ui.sliderfloat("Below HP (%)", self.threshold, 1, 100)
		
	def check(self, ctx, target, spell):
		return target.health/target.max_health < self.threshold/100.0
		
	def __str__(self):
		return json.dumps([self.threshold])

Conditions = {
	'KeyToggleCondition':      KeyToggleCondition,
	'KillableCondition':       KillableCondition,
	'HealthCondition':         HealthCondition,
	'SmitableCondition':       SmitableCondition,
	'KeyHoldCondition':        KeyHoldCondition
}		

class CastModeSelf:
	def __str__(self):
		return 'Self'
		
	def get_targets(self, ctx):
		return [ctx.player]
		
	def get_targeters(self):
		return TargetersChampion

class CastModeEnemyChamp:
	def __str__(self):
		return 'Enemy Champion'
		
	def get_targets(self, ctx):
		return filter(lambda x: x.enemy_to(ctx.player), ctx.champs)
		
	def get_targeters(self):
		return TargetersChampion
	
class CastModeMonster:
	def __str__(self):
		return 'Jungle Monster'
	
	def get_targets(self, ctx):
		return ctx.jungle
		
	def get_targeters(self):
		return TargetersMonster

class ConditionEvaluateModeBySpellName:
	
	def evaluate(self, conditions, ctx, target, spell):
		pass
		
	def __str__(self):
		return 'BySpellName match'


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

class Activator:
	
	All                       = {}
	BySpellName               = {}
	
	cast_modes        = [CastModeSelf(), CastModeEnemyChamp(), CastModeMonster()]
	cast_mode_current = 0
	
	target_selector = TargetSelector()
	spells          = {}
	
	def __init__(self, name, spells, conditions):
		self.name        = name
		self.spells      = spells
		self.conditions  = conditions
		

	def check(self, ctx, spell_name):
		
		if len(self.conditions) == 0:
			return None
			
		spell = self.spells.get(spell_name)	
		if not spell:
			raise Exception("Illegal activator.check() call for spell " + spell_name)
			
		target = self.get_target(ctx, spell)
		if target == None:
			return None
			
		for condition in self.conditions:
			if not condition.check(ctx, target, spell):
				return None
		
		return target
	
	def ui(self, ctx):
		global SummonerSpells
		ui = ctx.ui
		
		if ui.beginmenu(self.name):
			
			# Draw target spells
			ui.text("Target spells", Col.Yellow)
			for spell in self.spells.values():
				if ui.button("X###" + spell.name, Col(0.8, 0.2, 0.2, 1)):
					del self.spells[spell.name]
					Activator.BySpellName[spell.name].remove(spell)
					break
				
				ui.sameline()
				ui.image(spell.icon, Vec2(15, 15), Col.White)
				ui.sameline()
				ui.text(spell.name)
				
			if ui.button("Add summoner spell"):
				ui.openpopup("add_spell_summ")
			ui.sameline()
			if ui.button("Add champion spell"):
				ui.openpopup("add_spell_champ")
			
			if ui.beginpopup("add_spell_summ"):
				for name in SummonerSpells:
					spell = ctx.get_spell(name)
					if spell:
						ui.image(spell.icon, Vec2(15, 15), Col.White)
						ui.sameline()
						if ui.selectable(spell.name):
							add_spell(spell, self)
						
				ui.endpopup()
			
			if ui.beginpopup("add_spell_champ"):
				for i in range(4):
					spell = ctx.player.spells[i]
					ui.image(spell.static.icon, Vec2(15, 15), Col.White)
					ui.sameline()
					if ui.selectable(spell.static.name):
						add_spell(spell, self)
						
				ui.endpopup()
			
			# Draw condition settings
			ui.separator()
			ui.text("Conditions", Col.Yellow)
			
			if ui.button("Add Condition"):
				ui.openpopup("addt_popup")
			if ui.beginpopup("addt_popup"):
				self.draw_add_conditions(ui)
				ui.endpopup()
			
			ui.separator()
			for i, condition in enumerate(self.conditions):
				if ui.button("X###" + str(i), Col(0.8, 0.2, 0.2, 1)):
					del self.conditions[i]
					break
				ui.sameline()	
				ui.text(type(condition).__name__, Col.Purple)
				
				condition.ui(ctx)
				ui.separator()
			
			# Draw cast settings
			ui.text("Targeting", Col.Yellow)
			new_mode = ui.combo("Target type", self.cast_modes, self.cast_mode_current)
			if new_mode != self.cast_mode_current:
				self.cast_mode_current = new_mode		
				self.update_target_selector()
			self.target_selector.ui(ctx, "Target selector")
			ui.endmenu()

	def get_target(self, ctx, spell):
		return self.target_selector.get_target(ctx, self.cast_modes[self.cast_mode_current].get_targets(ctx), spell.cast_range + ctx.player.static.gameplay_radius)

	def update_target_selector(self):
		self.target_selector.targeters         = self.cast_modes[self.cast_mode_current].get_targeters()
		self.target_selector.selected_targeter = 0
		
	def draw_add_conditions(self, ui):
		global Conditions
		for s, c in Conditions.items():
			if ui.selectable(s):
				self.conditions.append(c())
	
	@classmethod
	def add_spell(self, spell, activator):
		activator.spells[spell.name] = spell
		if spell.name not in Activator.BySpellName:
			Activator.BySpellName[spell.name] = []
		Activator.BySpellName[spell.name].append(activator)

	def __str__(self):
		return json.dumps([self.name, list(self.spells.keys()), { type(condition).__name__ : str(condition) for condition in self.conditions }, str(self.target_selector), self.cast_mode_current])
		
	@classmethod
	def from_str(self, ctx, s):
		name, jspells, jconditions, jtargeter, current_targeter = json.loads(s)
		
		activator = Activator(name, {}, [])
		
		for name in jspells:
			spell = ctx.get_spell(name)
			if not spell:
				raise Exception("Couldnt find spell " + name)
			self.add_spell(spell, activator)
		
		conditions = []
		for condition_name, condition_str in jconditions.items():
			args = json.loads(condition_str)
			exec(f'conditions.append({condition_name}(*args))')
		
		activator.conditions        = conditions
		activator.target_selector   = TargetSelector.from_str(jtargeter)
		activator.cast_mode_current = current_targeter
		activator.update_target_selector()
		
		Activator.All[activator.name] = activator
		return activator


name_activator  = 'Name activator'
cast_timestamps = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0] # Used to make sure we dont cast a spell 100 times a second

def valkyrie_menu(ctx):
	global name_activator
	ui = ctx.ui
	
	name_activator = ui.inputtext("Activator name", name_activator)
	if ui.button("Add Activator"):
		if len(name_activator) > 0 and name_activator not in Activator.All:
			Activator.All[name_activator] = Activator(name_activator, {}, [])

	ui.text('Activators')
	to_rename = None
	for name, activator in Activator.All.items():
		
		if ui.button("X###d" + name, Col(0.8, 0.2, 0.2, 1)):
			del Activator.All[name]
			break
			
		ui.sameline()
		if ui.button("Rename###r" + name, Col(0.2, 0.6, 0.2, 1)):
			to_rename = name
			
		ui.sameline()
		activator.ui(ctx)

	if to_rename != None and to_rename != name_activator and len(name_activator) > 0 and name_activator not in Activator.All:
		Activator.All[to_rename].name = name_activator
		Activator.All[name_activator] = Activator.All[to_rename]
		del Activator.All[to_rename]

def valkyrie_on_load(ctx):
	cfg = ctx.cfg
	
	activator_names = json.loads(cfg.get_str("activator_names", "[]"))
	for name in activator_names:
		activator = Activator.from_str(ctx, cfg.get_str(name, str(Activator(name, {}, []))))

def valkyrie_on_save(ctx):
	cfg = ctx.cfg
	
	cfg.set_str("activator_names", json.dumps(list(Activator.All.keys())))
	for activator in Activator.All.values():
		cfg.set_str(activator.name, str(activator))

def valkyrie_exec(ctx):
	global cast_timestamps
	player = ctx.player
	
	for i, spell in enumerate(player.spells):
		if spell.cd > 0.0:
			continue
			
		now = time.time()
		if now - cast_timestamps[i] < 0.1:
			continue
			
		activators = Activator.BySpellName.get(spell.name, [])
		for activator in activators:
			target = activator.check(ctx, spell.name)
			if target:
				ctx.cast_spell(spell, target.pos)
				cast_timestamps[i] = now
				break