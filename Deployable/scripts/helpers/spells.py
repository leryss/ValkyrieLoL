import os, json
from enum import Enum

class CCType:
	Charm    = 0
	Stun     = 1
	Fear     = 2
	Slow     = 3
	Suppress = 4
	Root     = 5
	Taunt    = 6
	Drowsy   = 7
	Sleep    = 8
	
	Names = {
		Charm    : 'Charm',
		Stun     : 'Stun',     		
		Fear     : 'Fear',     		
		Slow     : 'Slow',     		
		Suppress : 'Suppress',
		Root     : 'Root',
		Taunt    : 'Taunt',
		Drowsy   : 'Drowsy',
		Sleep    : 'Sleep'
	}

class BuffType:
	Mastery = 1
	CC      = 2
	Potion  = 4

class Buff:
	
	def __init__(self, pretty_name, name, type, type_info = None):
		self.type = type
		self.type_info = type_info
		self.pretty_name = pretty_name
		self.name = name
		
	def is_type(self, type):
		return self.type & type == type

class Buffs:

	UnknownBuff = Buff('UnknownBuff', '?', 0)
	AllBuffs = [
		#    Pretty nam               In game name                 Type              More type info
		Buff('Charm',                'Charm',                      BuffType.CC,      CCType.Charm),
		Buff('Stun',                 'Stun',                       BuffType.CC,      CCType.Stun),
		Buff('Flee',                 'Flee',                       BuffType.CC,      CCType.Fear),
		Buff('Suppress',             'suppression',                BuffType.CC,      CCType.Suppress),
		Buff('Taunt',                'puncturingtauntattackspeed', BuffType.CC,      CCType.Taunt),
								     
		# Slows                                              
		Buff('Slow',                 'slow',                       BuffType.CC,      CCType.Slow),
		Buff('WaterDrakeSlow',       'waterdragonslow',            BuffType.CC,      CCType.Slow),
		Buff('AshePassiveSlow',      'ashepassiveslow',            BuffType.CC,      CCType.Slow),
								     						      
		# Roots                                                    
		Buff('JhinWRoot',            'JhinW',                      BuffType.CC,      CCType.Root),
		Buff('LuxQRoot',             'LuxLightBindingMis',         BuffType.CC,      CCType.Root),
							         
		# Masteries                  
		Buff('LethalTempo',          'ASSETS/Perks/Styles/Precision/LethalTempo/LethalTempoEmpowered.lua', BuffType.Mastery),
		 
		# Potions
		Buff('DarkCrystalFlask',     'ItemDarkCrystalFlask',        BuffType.Potion),
		Buff('CrystalFlask',         'ItemCrystalFlask',            BuffType.Potion),
		Buff('RedPot',               'Item2003',                    BuffType.Potion)
	]
	
	AllBuffsDict = { buff.pretty_name : buff for buff in AllBuffs } | { buff.name : buff for buff in AllBuffs }

	@classmethod
	def has_buff(self, champ, buff_name):
		buff_obj = Buffs.AllBuffsDict.get(buff_name, None)
		if buff_obj == None:
			return False
			
		return champ.has_buff(buff_obj.name)
	
	@classmethod
	def get(self, buff_name):
		return Buffs.AllBuffsDict.get(buff_name, Buffs.UnknownBuff)

'''
Spells = {}

def load_spell_calcs(path):
	global Spells
	
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
			
		Spells[name.lower()] = type('Spell' + name, (SpellBase, ), obj_dict)()

class SpellBase:
	def calculate_property(self, property_name, champ, skill):
		formula = getattr(self, property_name)
		self.Data['champ'] = champ
		self.Data['skill'] = skill
		return eval(formula, self.Data)
		

class ChampDummy:
	lvl = 10
	ap  = 1000
	ad  = 100
	
	bonus_armor     = 20
	bonus_atk_speed = 0.5
	bonus_magic_res = 20
	bonus_ms        = 100
	crit            = 0.5
	crit_multi      = 1.1
	haste_multi     = 0.3
	max_hp          = 1000
	hp              = 700
	lethality       = 20
	
class SkillDummy:
	lvl = 1
	
	
def get_spell_effective_dmg(champ, spell):
	spell_info = Spells.get(spell.name, None)
	if spell_info == None:
		return 0.0
		
	return spell_info.calculate_property()
'''