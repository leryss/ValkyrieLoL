import json
from valkyrie import *
from .targeting import TargetSelector
from .flags import Orbwalker
from .inputs import KeyInput
from .spells import Buffs, BuffType, SpellCondition
from .damages import calculate_raw_spell_dmg
import pickle, base64

class Enabler:

	enable_type = ['Always On', 'Key Input']

	def __init__(self, always_on, keyinput):
		self.keyinput  = keyinput
		self.selected  = 0 if always_on else 1
		
	def ui(self, ui, label = 'Activation Type'):
		self.selected = ui.sliderenum(label, self.enable_type[self.selected], self.selected, 1)
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
		
	@classmethod
	def default(self):
		return Enabler(False, KeyInput(0, False))
		
class IntAttribute:
	def __init__(self, name, minval, maxval, solver):
		self.name   = name
		self.solver = solver
		self.minval = minval
		self.maxval = maxval
		
	def ui(self, label, ui, val):
		return ui.sliderint(f'{label} {self.name}', val, self.minval, self.maxval)
		
class FloatAttribute:
	def __init__(self, name, minval, maxval, solver):
		self.name = name
		self.solver = solver
		self.minval = minval
		self.maxval = maxval
	
	def ui(self, label, ui, val):
		return ui.sliderfloat(f'{label} {self.name}', val, self.minval, self.maxval)

def attr_solve_mana(unit)       : return unit.mana
def attr_solve_health(unit)     : return unit.health
def attr_solve_armor(unit)      : return unit.armor
def attr_solve_magic_res(unit)  : return unit.magic_res
def attr_solve_atk(unit)        : return unit.atk
def attr_solve_lethality(unit)  : return unit.lethality
def attr_solve_haste(unit)      : return unit.haste
def attr_solve_cdr(unit)        : return unit.cdr
def attr_solve_base_atk(unit)   : return unit.base_atk
def attr_solve_move_speed(unit) : return unit.move_speed
def attr_solve_crit(unit)       : return unit.crit
def attr_solve_crit_multi(unit) : return unit.crit_multi
def attr_solve_ap(unit)         : return unit.ap
def attr_solve_atk_speed(unit)  : return unit.atk_speed
def attr_solve_atk_range(unit)  : return unit.atk_range   
def attr_solve_lvl(unit)        : return unit.lvl

class Attributes:
	Mana              = 0 
	Health            = 1 
	Armor             = 2 
	MagicResist       = 3 
	AD                = 4 
	Lethality         = 5 
	Haste             = 6 
	CooldownReduction = 7
	BaseAD            = 8       
	MoveSpeed         = 9 
	Crit	          = 10 
	CritMulti         = 11
	AP                = 12 
	AttackSpeed       = 13 
	AttackRange       = 14
	Level             = 15
	
	All = {
		Mana             : FloatAttribute('Mana'             , 0.0, 4000.0, attr_solve_mana)       ,
		Health           : FloatAttribute('Health'           , 0.0, 5000.0, attr_solve_health)     ,
		Armor            : FloatAttribute('Armor'            , 0.0, 400.0,  attr_solve_armor)      ,
		MagicResist      : FloatAttribute('MagicResist'      , 0.0, 400.0,  attr_solve_magic_res)  ,
		AD               : FloatAttribute('AD'               , 0.0, 400.0,  attr_solve_atk)        ,
		Lethality        : FloatAttribute('Lethality'        , 0.0, 100.0,  attr_solve_lethality)  ,
		Haste            : FloatAttribute('Haste'            , 0.0, 200.0,  attr_solve_haste)      ,
		CooldownReduction: FloatAttribute('CooldownReduction', 0.0, 70.0,   attr_solve_cdr)        ,
		BaseAD           : FloatAttribute('BaseAD'           , 0.0, 150.0,  attr_solve_base_atk)   ,
		MoveSpeed        : FloatAttribute('MoveSpeed'        , 0.0, 700.0,  attr_solve_move_speed) ,
		Crit	         : FloatAttribute('Crit'	          , 0.0, 1.0,    attr_solve_crit)       ,
		CritMulti        : FloatAttribute('CritMulti'        , 0.0, 2.0,    attr_solve_crit_multi) ,
		AP               : FloatAttribute('AP'               , 0.0, 1500.0, attr_solve_ap)         ,
		AttackSpeed      : FloatAttribute('AttackSpeed'      , 0.0, 5.0,    attr_solve_atk_speed)  ,
		AttackRange      : FloatAttribute('AttackRange'      , 0.0, 1000.0, attr_solve_atk_range)  ,
		Level            : IntAttribute('Level'              , 1, 18,       attr_solve_lvl)      
	}
	
	AllNames = [a.name for a in All.values()]

class ChampionScript:
	
	Version = '0.2 alpha'
	
	def __init__(self, passive_trigger, combat_rotation, passive_rotation, combat_distance = None, passive_distance = None):
		self.passive_trigger = passive_trigger
		self.combat_rotation = combat_rotation
		self.passive_rotation = passive_rotation
		
	def ui(self, ctx):
		ui = ctx.ui
	
		ui.text(f'Powered by Valkyrie Champion Script v{self.Version}', Col.Yellow)

		if ui.beginmenu('Orbwalker spell rotation'):
			self.combat_rotation.ui(ctx, ui)
			ui.endmenu()
		ui.help('This spell rotation is made for combat using orbwalker')
		
		if ui.beginmenu('Passive spell rotation'):
			self.passive_rotation.ui(ctx, ui)
			ui.endmenu()
		ui.help('This spell rotation is made for passive/sniping etc')
			
		self.passive_trigger.ui(ui, 'Passive rotation')
		
	def exec(self, ctx):
		player = ctx.player
		if Orbwalker.CurrentMode:
			if Orbwalker.CurrentMode == Orbwalker.ModeKite:
				ctx.pill('SpellKite', Col.Green, Col.Black)
				self.combat_rotation.cast(ctx, Orbwalker.SelectorChampion)
				return
			
		if self.passive_trigger.check(ctx) and Orbwalker.ModeKite:
			ctx.pill('Passive', Col.Purple, Col.Black)
			self.passive_rotation.cast(ctx, Orbwalker.SelectorChampion)

		
	@classmethod
	def from_str(self, s):
		result = pickle.loads(base64.b64decode(s))
		if ChampionScript.Version != result.Version:
			raise Exception('Champion script settings are outdated. Please "Reset to Defaults"')
		return result
		
	def __str__(self):
		return base64.b64encode(pickle.dumps(self)).decode('ascii')
		
class ConditionInFrontOfTarget(SpellCondition):
	def _check(self, ctx, player, target, spell):
		return player.in_front_of(target)
		
	def _get_name(self):
		return 'Player in front of target'
	
	def _get_help(self):
		return 'Triggers when player is in front of target'
	
class ConditionTargetPoisoned(SpellCondition):
	def _check(self, ctx, player, target, spell):
		return Buffs.has_buff_type(target, BuffType.Poison)
		
	def _get_name(self):
		return 'Is poisoned'
		
	def _get_help(self):
		return 'Triggers when target is poisoned'
		
class ConditionIncomingMissiles(SpellCondition):

	def __init__(self, time_until_impact):
		self.time_until_impact = time_until_impact
		self.enabled = True
		
	def _check(self, ctx, player, target, spell):
		cols = ctx.collisions_for(player)
		for col in cols:
			if col.time_until_impact < self.time_until_impact:
				return True
				
		return False
		
	def _get_name(self):
		return 'Skillshot impact'
		
	def _get_help(self):
		return 'Triggers when missile (skillshots) are going to collide with the player'
		
	def _ui(self, ctx, ui):
		self.time_until_impact = ui.sliderfloat('Time until collision', self.time_until_impact, 0.2, 1.0)
		
class ConditionTargetOutsideTower(SpellCondition):

	def _check(self, ctx, player, target, spell):
		return not ctx.is_under_tower(target)
		
	def _get_name(self):
		return 'Target outside turret range'
		
	def _get_help(self):
		return 'Triggers when target is outside a turret range'
		
class ConditionDistanceToTarget(SpellCondition):
	
	def __init__(self, distance_min, distance_max):
		self.distance_min = distance_min
		self.distance_max = distance_max
		self.enabled = True
		
	def _check(self, ctx, player, target, spell):
		dist = player.pos.distance(target.pos)
		return dist > self.distance_min and dist < self.distance_max
		
	def _get_name(self):
		return 'Distance to target'
		
	def _get_help(self):
		return 'Triggers when distance is between two values'
		
	def _ui(self, ctx, ui):
		self.distance_min = ui.sliderfloat('Distance Atleast', self.distance_min, 0.0, 3000.0)
		self.distance_max = ui.sliderfloat('Distance Atmost',  self.distance_max, 0.0, 3000.0)
		
class ConditionTargetHPBelow(SpellCondition):
	
	def __init__(self, hp):
		self.hp = hp
		self.enabled = True
		
	def _check(self, ctx, player, target, spell):
		return target.health/target.max_health < float(self.hp)/100.0
		
	def _get_name(self):
		return 'Health of target'
		
	def _get_help(self):
		return 'Triggers when health of the target is below % HP'

	def _ui(self, ctx, ui):
		self.hp = ui.sliderint('HP % below', self.hp, 1, 99)

class ConditionAttribute(SpellCondition):
	
	def __init__(self, attr, min_attr, max_attr):
		self.min_attr = min_attr
		self.max_attr = max_attr
		self.attr     = attr
		self.enabled  = True
	
	def _check(self, ctx, player, target, spell):
		val = Attributes.All[self.attr].solver(target)
		return val >= self.min_attr and val <= self.max_attr
		
	def _get_name(self):
		return 'Target Attribute'
		
	def _ui(self, ctx, ui):
		self.attr = ui.combo('Attribute', Attributes.AllNames, self.attr)
		
		attr = Attributes.All[self.attr]
		self.min_attr = attr.ui('Atleast', ui, self.min_attr)
		self.max_attr = attr.ui('Atmost', ui, self.max_attr)
		
	def _get_help(self):
		return 'Condition that checks a attribute value of the target and checks if its within bounds'

class ConditionKillable(SpellCondition):
	
	def _check(self, ctx, player, target, spell):
		dmg = calculate_raw_spell_dmg(player, spell)
		return dmg.calc_against(ctx, player, target) > target.health
		
	def _get_name(self):
		return 'Killable Target'
		
	def _get_help(self):
		return 'Triggers if the target is killable'
		
def condition_solver_all(conditions, ctx, player, target, spell):
	for c in conditions:
		if not c.check(ctx, player, target, spell):
			return False
	return True
	
def condition_solver_any(conditions, ctx, player, target, spell):
	for c in conditions:
		if c.check(ctx, player, target, spell):
			return True
	return False
		
class MixedConditions(SpellCondition):

	All = 0
	Any = 1
	Solvers = {
		All: condition_solver_all,
		Any: condition_solver_any
	}
	SolversStr = [
		'All Satisfied', 'Any Satisfied'
	]

	def __init__(self, conditions, cmode):
		self.conditions = conditions
		self.cmode = cmode
		self.enabled = True
		
	def _check(self, ctx, player, target, spell):
		return self.Solvers.get(self.cmode, condition_solver_all)(self.conditions, ctx, player, target, spell)
				
	def _get_name(self):
		return 'Mixed'
		
	def _get_help(self):
		return 'This Condition is Mixed. That means its made from multiple conditions governed by a rule. Example of rules are: Any - any child condition needs to be True, All - all child conditions must be true'
		
	def _ui(self, ctx, ui, depth = 0):
	
		self.cmode = ui.sliderenum('Rule', self.SolversStr[self.cmode], self.cmode, 1)

		ui.indent(10.0 * depth)
		ui.separator()
		for c in self.conditions:
			ui.pushid(id(c))
			c.ui(ctx, ui, depth + 1)
			ui.popid()