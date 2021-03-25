from valkyrie import *		
import os, json

Calculations = {}

class Damage:
	def __init__(self, raw_dmg):
		self.raw_dmg = raw_dmg
	
	def calc_against(self, ctx, attacker, target):
		return 0.0
		
	def get_color(self, alpha = 1.0):
		col = self._color()
		return Col(col.r, col.g, col.b, alpha)
		
	def _color(self):
		return Col.Gray
	
class MagicDamage(Damage):
	def calc_against(self, ctx, attacker, target):
		return attacker.effective_magic_dmg(target, self.raw_dmg)
		
	def _color(self):
		return Col.Cyan

class PhysDamage(Damage):
	def calc_against(self, ctx, attacker, target):
		return attacker.effective_phys_dmg(target, self.raw_dmg)

	def _color(self):
		return Col.Orange

class TrueDamage(Damage):
	def calc_against(self, ctx, attacker, target):
		return self.raw_dmg

	def _color(self):
		return Col.White
		
class WrapMaxHP(Damage):
	def __init__(self, dmg_applied):
		self.dmg_applied = dmg_applied
	
	def calc_against(self, ctx, attacker, target):
		old_dmg = self.dmg_applied.raw_dmg
		
		self.dmg_applied.raw_dmg *= target.max_health
		result = self.dmg_applied.calc_against(ctx, attacker, target)
		
		self.dmg_applied.raw_dmg = old_dmg
		
		return result

	def _color(self):
		return self.dmg_applied._color()

class MixedDamage(Damage):
	def __init__(self, dmg_list):
		self.dmg_list = dmg_list
		
	def calc_against(self, ctx, attacker, target):
		return sum([dmg.calc_against(ctx, attacker, target) for dmg in self.dmg_list])
	
	def _color(self):
		return Col.Red
		
class TwitchExpungeDamage(MixedDamage):
	def __init__(self, base, phys, magic):
		self.base = base
		self.phys = phys
		self.magic = magic
		
	def calc_against(self, ctx, attacker, target):
		stacks = target.num_buff_stacks('TwitchDeadlyVenom')
		
		return self.base.calc_against(ctx, attacker, target) + stacks*self.phys.calc_against(ctx, attacker, target) + stacks*self.magic.calc_against(ctx, attacker, target)

class CassiopeiaEDamage(MagicDamage):
	def __init__(self, base, bonus):
		self.base = base
		self.bonus = bonus
		
	def calc_against(self, ctx, attacker, target):
		
		dmg = self.base.calc_against(ctx, attacker, target)
		
		if type(target) == ChampionObj:
			poisoned = target.has_buff('cassiopeiaqdebuff') or target.has_buff('cassiopeiawbuff')
			if poisoned:
				dmg += self.bonus.calc_against(ctx, attacker, target)
		
		return dmg

class KogMawRDamage(MagicDamage):
	
	def calc_against(self, ctx, attacker, target):
		p = target.health/target.max_health
		if p < 0.4:
			self.raw_dmg *= 2.0
			dmg = super().calc_against(ctx, attacker, target)
			self.raw_dmg /= 2.0
		else:
			dmg = super().calc_against(ctx, attacker, target)
		
		return dmg
		
class DariusRDamage(TrueDamage):
	
	def __init__(self, base):
		self.base = base
	
	def calc_against(self, ctx, attacker, target):
		num_stacks = target.num_buff_stacks('DariusHemo')
		return self.base + self.base * (0.2*num_stacks)
		
class SyndraRDamage(MagicDamage):
	
	def __init__(self, base):
		self.base = base
	
	def calc_against(self, ctx, attacker, target):
		self.raw_dmg = self.base * min(7, 3 + len(ctx.others.has_tag(Unit.SpecialSyndraSphere).alive().get()))
		
		return super().calc_against(ctx, attacker, target)
		
		
class TristanaEDamage(MagicDamage):
	def __init__(self, base, bonus_per_stack):
		self.base = base
		self.bonus_per_stack = bonus_per_stack
	
	def calc_against(self, ctx, attacker, target):
		self.raw_dmg = self.base + self.base * 1.2
		self.raw_dmg = self.raw_dmg + self.raw_dmg * (0.033*attacker.crit/0.1)
		
		return super().calc_against(ctx, attacker, target)
		
class DianaRDamage(MagicDamage):
	def __init__(self, base, bonus_per_champ):
		self.base = base
		self.bonus_per_champ = bonus_per_champ
	
	def calc_against(self, ctx, attacker, target):
		num_champs = len(ctx.champs.enemy_to(attacker).targetable().near(attacker, 475).get())
		self.raw_dmg = self.base + num_champs * self.bonus_per_champ
		return super().calc_against(ctx, attacker, target)
		
class JinxRDmage(PhysDamage):
	
	missing_hp = [0.25, 0.30, 0.35]
	def __init__(self, base):
		self.base = base
		
	def calc_against(self, ctx, attacker, target):
		
		lvl = max(0, attacker.spells[3].lvl - 1)
		mhp = self.missing_hp[lvl]
		multi = min(1.0, 0.1 + 0.06 * attacker.pos.distance(target.pos) / 100.0)
		
		self.raw_dmg = self.base*multi + mhp*(target.max_health - target.health)
		return super().calc_against(ctx, attacker, target)
		
DamageExtractors = {
	
	# Ahri
	'ahriorbofdeception'     : lambda calc, champ, skill: MixedDamage([MagicDamage(calc.totaldamage(champ, skill)), TrueDamage(calc.totaldamage(champ, skill))]),
	'ahrifoxfire'            : lambda calc, champ, skill: MagicDamage(calc.singlefiredamage(champ, skill) + calc.multifiredamage(champ, skill)*2.0),
	'ahriseduce'             : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'ahritumble'             : lambda calc, champ, skill: MagicDamage(calc.rcalculateddamage(champ, skill) * 3.0),
	
	# Darius
	'dariuscleave'           : lambda calc, champ, skill: PhysDamage(calc.bladedamage(champ, skill)),
	'dariusnoxiantacticsonh' : lambda calc, champ, skill: PhysDamage(calc.empoweredattackdamage(champ, skill)),
	'dariusexecute'          : lambda calc, champ, skill: DariusRDamage(calc.damage(champ, skill)),
	
	# Diana
	'dianaq'                 : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'dianaorbs'              : lambda calc, champ, skill: MagicDamage(calc.totalmaxdamage(champ, skill)),
	'dianateleport'          : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'dianar'                 : lambda calc, champ, skill: DianaRDamage(calc.rexplosiondamage(champ, skill), calc.rmultihitamplification(champ, skill)),
	
	# Ezreal
	'ezrealq'                : lambda calc, champ, skill: PhysDamage(calc.damage(champ, skill)),
	'ezrealw'                : lambda calc, champ, skill: MagicDamage(calc.damage(champ, skill)),
	'ezreale'                : lambda calc, champ, skill: MagicDamage(calc.damage(champ, skill)),
	'ezrealr'                : lambda calc, champ, skill: MagicDamage(calc.damage(champ, skill)),
	
	# Fiora
	'fioraq'                 : lambda calc, champ, skill: PhysDamage(calc.totaldamage(champ, skill)),
	'fioraw'                 : lambda calc, champ, skill: MagicDamage(calc.stabdamage(champ, skill)),
	'fiorar'                 : lambda calc, champ, skill: WrapMaxHP(TrueDamage(4.0 * Calculations['fiorapassive'].passivedamagetotal(champ, skill))),
	
	# Irelia
	'ireliaq'                : lambda calc, champ, skill: PhysDamage(calc.championdamage(champ, skill)),
	'ireliaw'                : lambda calc, champ, skill: PhysDamage(calc.maxdamagecalc(champ, skill)),
	'ireliae'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'ireliar'                : lambda calc, champ, skill: MagicDamage(calc.missiledamage(champ, skill)),
	
	# Jinx
	'jinxw'                  : lambda calc, champ, skill: PhysDamage(calc.totaldamage(champ, skill)),
	'jinxe'                  : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'jinxr'                  : lambda calc, champ, skill: JinxRDmage(calc.damagemax(champ, skill)),
	
	# Kayle
	'kayleq'                 : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'kayler'                 : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	
	# Cassiopeia
	'cassiopeiaq'            : lambda calc, champ, skill: MagicDamage(calc.tooltiptotaldamage(champ, skill)),
	'cassiopeiaw'            : lambda calc, champ, skill: MagicDamage(calc.damagepersecond(champ, skill)*5.0),
	'cassiopeiae'            : lambda calc, champ, skill: CassiopeiaEDamage(MagicDamage(calc.totaldamage(champ, skill)), MagicDamage(calc.bouspoisoneddamage(champ, skill))),
	'cassiopeiar'            : lambda calc, champ, skill: MagicDamage(calc.rdamage(champ, skill)),
	
	# Kogmaw
	'kogmawq'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'kogmawvoidooze'         : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'kogmawlivingartillery'  : lambda calc, champ, skill: KogMawRDamage(calc.basedamagecalc(champ, skill)),
					          				
	# Samira                 
	'samiraq'                : lambda calc, champ, skill: PhysDamage(calc.damagecalc(champ, skill)),
	'samiraw'                : lambda calc, champ, skill: PhysDamage(calc.damagecalc(champ, skill)),
	'samirae'                : lambda calc, champ, skill: MagicDamage(calc.dashdamage(champ, skill)),
	'samirar'                : lambda calc, champ, skill: PhysDamage(11.0 * calc.damagecalc(champ, skill)),
	
	# Soraka
	'sorakaq'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'sorakae'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)*2.0),
	
	# Syndra
	'syndraq'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'syndraw'                : lambda calc, champ, skill: MagicDamage(calc.throwdamage(champ, skill)),
	'syndrae'                : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'syndrar'                : lambda calc, champ, skill: SyndraRDamage(calc.damagecalc(champ, skill)),
	
	# Teemo
	'blindingdart'           : lambda calc, champ, skill: MagicDamage(calc.calculateddamage(champ, skill)),
	'teemorcast'             : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	
	# Tristana
	'tristanaw'              : lambda calc, champ, skill: MagicDamage(calc.landingdamage(champ, skill)),
	'tristanae'              : lambda calc, champ, skill: TristanaEDamage(calc.activedamage(champ, skill), calc.activemaxdamage(champ, skill)),
	'tristanar'              : lambda calc, champ, skill: MagicDamage(calc.damagecalc(champ, skill)),
	
	# Twitch
	'twitchexpunge'          : lambda calc, champ, skill: TwitchExpungeDamage(PhysDamage(calc.basedamage[skill.lvl - 1]), PhysDamage(calc.physicaldamageperstack(champ, skill)), MagicDamage(calc.magicdamageperstack(champ, skill))),
	
	# Urgot
	'urgotq'                 : lambda calc, champ, skill: PhysDamage(calc.totaldamage(champ, skill)),
	'urgotw'                 : lambda calc, champ, skill: PhysDamage(12.0 * calc.damagepershot(champ, skill)),
	'urgote'                 : lambda calc, champ, skill: PhysDamage(calc.edamage(champ, skill)),
	'urgotr'                 : lambda calc, champ, skill: PhysDamage(calc.rcalculateddamage(champ, skill)),
	
	# Yasuo
	'yasuoq1wrapper'         : lambda calc, champ, skill: PhysDamage(calc.totaldamagecrit(champ, skill)),
	'yasuoq2wrapper'         : lambda calc, champ, skill: PhysDamage(calc.totaldamagecrit(champ, skill)),
	'yasuoq3wrapper'         : lambda calc, champ, skill: PhysDamage(calc.totaldamagecrit(champ, skill)),
	'yasuoe'                 : lambda calc, champ, skill: MagicDamage(calc.totaldamage(champ, skill)),
	'yasuor'                 : lambda calc, champ, skill: PhysDamage(calc.damage(champ, skill)),
}

DuplicateMap = {
	'yasuoq1wrapper': ['yasuoq2wrapper', 'yasuoq3wrapper'],
}

def load_spell_calcs(path):
	global Calculations
	
	j = {}
	with open(path, 'r') as f:
		j = json.loads(f.read())

	
	for name, vdict in j.items():
		lname = name.lower()
		
		obj_dict = {}
		for dval_name, dval_values in vdict['data_vals'].items():
			obj_dict[dval_name] = dval_values
			
		for formula_name, formula_str in vdict['calcs'].items():
			if '{' not in formula_str and '{' not in formula_name:
				exec(f'def {formula_name}(self, champ, skill): return {formula_str}')
				obj_dict[formula_name] = eval(formula_name)
			
		obj = type('spell_' + lname, (object, ), obj_dict)()
		Calculations[lname] = obj
		if lname in DuplicateMap:
			for name in DuplicateMap[lname]:
				Calculations[name] = obj

def calculate_raw_spell_dmg(champ, skill):
	calculations = Calculations.get(skill.name, None)
	if calculations == None:
		return Damage(0.0)
	
	extractor = DamageExtractors.get(skill.name, None)
	if extractor == None:
		return Damage(0.0)
		
	return extractor(calculations, champ, skill)
	
	
load_spell_calcs(os.path.join(os.getenv('APPDATA'), 'Valkyrie\data\SpellCalculations.json'))