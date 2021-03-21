from valkyrie import *		
import os, json

Calculations = {}

class Damage:
	def __init__(self, raw_dmg):
		self.raw_dmg = raw_dmg
	
	def calc_against(self, attacker, target):
		return 0.0
		
	def get_color(self, alpha = 1.0):
		col = self._color()
		return Col(col.r, col.g, col.b, alpha)
		
	def _color(self):
		return Col.Gray
	
class MagicDamage(Damage):
	def calc_against(self, attacker, target):
		return attacker.effective_magic_dmg(target, self.raw_dmg)
		
	def _color(self):
		return Col.Cyan

class PhysDamage(Damage):
	def calc_against(self, attacker, target):
		return attacker.effective_phys_dmg(target, self.raw_dmg)

	def _color(self):
		return Col.Orange

class TrueDamage(Damage):
	def calc_against(self, attacker, target):
		return self.raw_dmg

	def _color(self):
		return Col.White

class MixedDamage(Damage):
	def __init__(self, dmg_list):
		self.dmg_list = dmg_list
		
	def calc_against(self, attacker, target):
		return sum([dmg.calc_against(attacker, target) for dmg in self.dmg_list])
	
	def _color(self):
		return Col.Red
		
class TwitchExpungeDamage(MixedDamage):
	def __init__(self, base, phys, magic):
		self.base = base
		self.phys = phys
		self.magic = magic
		
	def calc_against(self, attacker, target):
		stacks = target.num_buff_stacks('TwitchDeadlyVenom')
		
		return self.base.calc_against(attacker, target) + stacks*self.phys.calc_against(attacker, target) + stacks*self.magic.calc_against(attacker, target)

class CassiopeiaEDamage(MagicDamage):
	def __init__(self, base, bonus):
		self.base = base
		self.bonus = bonus
		
	def calc_against(self, attacker, target):
		
		dmg = self.base.calc_against(attacker, target)
		
		if type(target) == ChampionObj:
			poisoned = target.has_buff('cassiopeiaqdebuff') or target.has_buff('cassiopeiawbuff')
			if poisoned:
				dmg += self.bonus.calc_against(attacker, target)
		
		return dmg

class KogMawRDamage(MagicDamage):
	
	def calc_against(self, attacker, target):
		p = target.health/target.max_health
		if p < 0.4:
			self.raw_dmg *= 2.0
			dmg = super().calc_against(attacker, target)
			self.raw_dmg /= 2.0
		else:
			dmg = super().calc_against(attacker, target)
		
		return dmg
		
DamageExtractors = {
	
	# Ahri
	'ahriorbofdeception'     : lambda calc, champ, skill: MixedDamage([MagicDamage(calc.totaldamage(champ, skill)), TrueDamage(calc.totaldamage(champ, skill))]),
	'ahrifoxfire'            : lambda calc, champ, skill: MagicDamage(calc.singlefiredamage(champ, skill) + calc.multifiredamage(champ, skill)*2.0),
	'ahriseduce'             : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'ahritumble'             : lambda calc, champ, skill: MagicDamage(calc.rcalculateddamage(champ, skill) * 3.0),
	
	# Darius
	'dariuscleave'           : lambda calc, champ, skill: PhysDamage(calc.bladedamage(champ, skill)),
	'dariusnoxiantacticsonh' : lambda calc, champ, skill: PhysDamage(calc.empoweredattackdamage(champ, skill)),
	'dariusexecute'          : lambda calc, champ, skill: TrueDamage(calc.maximumdamage(champ, skill)),
	
	# Cassiopeia
	'cassiopeiaq'            : lambda calc, champ, skill: MagicDamage(calc.tooltiptotaldamage(champ, skill)),
	'cassiopeiaw'            : lambda calc, champ, skill: MagicDamage(calc.damagepersecond(champ, skill)*5.0),
	'cassiopeiae'            : lambda calc, champ, skill: CassiopeiaEDamage(MagicDamage(calc.totaldamage(champ, skill)), MagicDamage(calc.bouspoisoneddamage(champ, skill))),
	'cassiopeiar'            : lambda calc, champ, skill: MagicDamage(calc.rdamage(champ, skill)),
	
	# Kogmaw
	'kogmawq'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'kogmawvoidooze'         : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'kogmawlivingartillery'  : lambda calc, champ, skill: KogMawRDamage(calc.basedamagecalc(champ, skill)),
	
	# Twitch
	'twitchexpunge'          : lambda calc, champ, skill: TwitchExpungeDamage(PhysDamage(calc.basedamage[skill.lvl - 1]), PhysDamage(calc.physicaldamageperstack(champ, skill)), MagicDamage(calc.magicdamageperstack(champ, skill))),
					          				
	# Samira                 
	'samiraq'                : lambda calc, champ, skill: PhysDamage(calc.damagecalc(champ, skill)),
	'samiraw'                : lambda calc, champ, skill: PhysDamage(calc.damagecalc(champ, skill)),
	'samirae'                : lambda calc, champ, skill: MagicDamage(calc.dashdamage(champ, skill)),
	'samirar'                : lambda calc, champ, skill: PhysDamage(11.0 * calc.damagecalc(champ, skill)),
}

def load_spell_calcs(path):
	global Calculations
	
	j = {}
	with open(path, 'r') as f:
		j = json.loads(f.read())

	
	for name, vdict in j.items():
		
		obj_dict = {}
		for dval_name, dval_values in vdict['data_vals'].items():
			obj_dict[dval_name] = dval_values
			
		for formula_name, formula_str in vdict['calcs'].items():
			if '{' not in formula_str and '{' not in formula_name:
				exec(f'def {formula_name}(self, champ, skill): return {formula_str}')
				obj_dict[formula_name] = eval(formula_name)
			
		Calculations[name.lower()] = type('Spell' + name, (object, ), obj_dict)()

def calculate_raw_spell_dmg(champ, skill):
	calculations = Calculations.get(skill.name, None)
	if calculations == None:
		return Damage(0.0)
	
	extractor = DamageExtractors.get(skill.name, None)
	if extractor == None:
		return Damage(0.0)
		
	return extractor(calculations, champ, skill)
	
	
load_spell_calcs(os.path.join(os.getenv('APPDATA'), 'Valkyrie\data\SpellCalculations.json'))