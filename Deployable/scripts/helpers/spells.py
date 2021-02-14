import os, json

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

load_spell_calcs(os.path.join(os.getenv('VPath'), 'data\\SpellCalculations.json'))

print(Spells['garene'])
x = Spells['garene'].TotalDamage(ChampDummy(), SkillDummy())
print(x)