import sys, os, json
from pprint import pprint, pformat
from math import floor

CalculationTranslators = {
	'NamedDataValueCalculationPart'              : lambda part: 'self.' + part['mDataValue'].replace(' ', '_') + '[skill.lvl]',
	'StatByNamedDataValueCalculationPart'        : lambda part: stat_calculation_translator(part, True),
	'StatByCoefficientCalculationPart'           : lambda part: stat_calculation_translator(part, False),
	'StatBySubPartCalculationPart'               : lambda part: stat_calculation_translator(part, False, part['mSubpart']),
	'NumberCalculationPart'                      : lambda part: str(part.get('mNumber', 0.0)),
	'ByCharLevelInterpolationCalculationPart'    : lambda part: f"{part.get('mStartValue', 0.0)} + champ.lvl * {(part['mEndValue'] - part.get('mStartValue', 0.0))/18.0}",
	'ByCharLevelBreakpointsCalculationPart'      : lambda part: by_level_breakpoint_translator(part),
	'EffectValueCalculationPart'                 : lambda part: f"self.Effect{part['mEffectIndex'] - 1}[skill.lvl]",
	'BuffCounterByNamedDataValueCalculationPart' : lambda part: f"self.{part['mDataValue'].replace(' ', '_')}[skill.lvl] * champ.num_buffs({part['mBuffName']})",
	'BuffCounterByCoefficientCalculationPart'    : lambda part: f"{part['mCoefficient']} * champ.num_buffs({part['mBuffName']})",
	'ProductOfSubPartsCalculationPart'           : lambda part: product_of_subparts_translator(part),
	'SumOfSubPartsCalculationPart'               : lambda part: sum_of_subparts_translator(part),
	'CooldownMultiplierCalculationPart'          : lambda part: 'champ.haste_multi',
	'AbilityResourceByCoefficientCalculationPart': lambda part: f"champ.max_mana * {part['mCoefficient']}",
	'ByCharLevelFormulaCalculationPart'          : lambda part: by_level_formula(part),
}

CurrentDataVals    = None
PrecalculatedIndex = 0

SpellCalculationFormulaExceptions = {
	'P + 08': 'P + 8'
}

StatTranslator = {
	-1000 : 'champ.ap',
	1     : 'champ.bonus_armor',
	2     : 'champ.ad',
	3     : 'champ.bonus_atk_speed',
	4     : 'champ.bonus_magic_res',
	5     : 'champ.bonus_ms',
	6     : 'champ.crit',
	7     : 'champ.crit_multi',
	8     : 'champ.haste_multi',
	10    : 'champ.max_hp',
	11    : 'champ.hp',
	23    : 'champ.lethality'
}

def by_level_formula(part):
	global PrecalculatedIndex, CurrentDataVals
	
	vals = []
	P = part.get('mBaseP', 0.0)
	formula = eval('lambda P, N: ' + part.get('mFormula', 'P'))
	
	for i in range(18):
		N = i
		P = formula(P, N)
		vals.append(P)
		
	key = 'PreCalculated' + str(PrecalculatedIndex)
	CurrentDataVals[key] = vals
	PrecalculatedIndex += 1
	
	return f"self.{key}[champ.lvl]"
	
def by_level_breakpoint_translator(part):
	global PrecalculatedIndex, CurrentDataVals
	formula = str(part.get('mLevel1Value', 0.0)) + ' {}'
	
	add_val = part.get('{02deb550}', 0.0)
	if 'mBreakpoints' in part:
		for point in part['mBreakpoints']:
			if '{d5fd07ed}' in point:
				val = point['{d5fd07ed}']
				lvl = point.get('mLevel', 1)
				formula = formula.format(f' + ({val} if LVL > {lvl - 1} else 0.0) {{}}')
			elif '{57fdc438}' in point:
				lvl = point.get('mLevel', 1)
				formula = formula.format(f'+ (LVL * {add_val} if LVL < {lvl} else {{}})')
				add_val = point['{57fdc438}']
				
		if 'else {}' in formula:
			formula = formula.format('0.0')
		else:
			formula = formula.format('')
	else:
		formula = formula.format(f'+ LVL * {add_val}')
	
	f = eval('lambda LVL:' + formula)
	vals = [f(i) for i in range(18)]
		
	key = 'PreCalculated' + str(PrecalculatedIndex)
	CurrentDataVals[key] = vals
	PrecalculatedIndex += 1
	
	return f'self.{key}[champ.lvl]'
	
def product_of_subparts_translator(part):
	part1 = part['mPart1']
	part2 = part['mPart2']
	
	return f"({CalculationTranslators[part1['__type']](part1)}) * ({CalculationTranslators[part2['__type']](part2)})"

def sum_of_subparts_translator(part):
	
	subpart_vals = []
	for subpart in part['mSubparts']:
		subpart_vals.append(f"({CalculationTranslators[subpart['__type']](subpart)})")
		
	return ' + '.join(subpart_vals)

def stat_calculation_translator(part, is_named, subpart = None):
	id_stat = part.get('mStat', -1000)
	stat = StatTranslator[id_stat]
	
	if subpart != None:
		val = f"({CalculationTranslators[subpart['__type']](subpart)})"
	else:
		val = 'self.' + part['mDataValue'].replace(' ', '_') + '[skill.lvl]' if is_named else part['mCoefficient']
	return f" ({val} * {stat})"

def get_part_value(part):
	return CalculationTranslators[part['__type']](part)

def perform_spell_calculations(spell):
	global CurrentDataVals, PrecalculatedIndex
	
	jdata_vals = spell.get("mDataValues",   [])
	jeffects   = spell.get("mEffectAmount", [])
	

	# Read data values
	data_vals = {}
	for jval in jdata_vals:
		vals = []
		formula = jval.get('mFormula', 'P')
		
		if formula in SpellCalculationFormulaExceptions:
			formula = SpellCalculationFormulaExceptions[formula]
		
		values = jval.get('mValues', [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
		for i, val in enumerate(values):
			N = i
			P = val
			vals.append(eval(formula))
		
		data_vals[jval['mName'].replace(' ', '_')] = vals
		
	for i, jeffect in enumerate(jeffects):
		values = jeffect.get('value', [])
		if len(values) > 0:
			data_vals['Effect' + str(i)] = [v for v in values]
		
	CurrentDataVals    = data_vals
	PrecalculatedIndex = 0
		
	# Read calculations
	calcs = {}
	jcalcs = spell.get('{94572284}', {})
	post_process = []
	for name, jcalc in jcalcs.items():
	
		_type = jcalc['__type']
		final_formula = ''
		if _type == 'GameCalculation':
			formulas = jcalc['mFormulaParts']
			for i, formula in enumerate(formulas):
				if i > 0:
					final_formula += " + "
				final_formula += get_part_value(formula)
		elif _type == 'GameCalculationModified':
			name_multiplied = jcalc['mModifiedGameCalculation']
			part_val        = get_part_value(jcalc['mMultiplier'])
			
			post_process.append((name, name_multiplied, part_val))
		else:
			print("Unknown game calculation " + _type)
			continue
			#raise Exception("Unknown game calculation " + _type)
			
		calcs[name] = final_formula.strip()
	
	for name, name_multiplied, part_multiplier in post_process:
		calcs[name] = f"({calcs[name_multiplied]}) * {part_multiplier}".strip()
			
	return {
		'data_vals': data_vals,
		'calcs':     calcs
	}

'''
	Generates two JSONs. One with unit data and one with spell data for unit spells. It extracts this data from the unit data downloaded with DownloadUnitData.py.
'''
def extract_unit_info(folder):

	def find_key_ending_with(dictionary, partial_key):
		for key, val in dictionary.items():
			if key.endswith(partial_key):
				return val
		return None
	
	units, spells = {}, {}
	spell_calculations = {}
	infos = ''
	unit_tags = set()
	

	for i, fname in enumerate(os.listdir(folder)):
		print("Processing: "+ fname)
		if fname.startswith(('brush_', 'nexusblitz_', 'slime_', 'tft4', 'tft_', 'sru_camprespawnmarker', 'preseason', 'test', 's5test')):
			print('Object blacklisted. Skipping...')
			continue
			
		props = {}
		with open(os.path.join(folder, fname)) as file:
			props = json.loads(file.read())
		
		# Find character property node
		root = find_key_ending_with(props, '/Root')	
		if not root:
			print('[Fail] No root found for: ' + fname)
			continue
		
		# Get character name
		name = root.get('mCharacterName', '')
		if len(name) == 0:
			print('[Fail] No character name found for: ' + fname)
			continue
			
		# Get basic attack info
		ba_windup	= 0.0
		ba_cast_time = 0.0
		ba_name	  = "?"
					
		if 'basicAttack' in root:
			basic_attack = root['basicAttack']
			ba_name	  = basic_attack.get("mAttackName", name + "basicattack")
			ba_cast_time = basic_attack.get("mAttackCastTime", 0.0)
			if ba_cast_time == 0:
				ba_cast_time  = 0.5 + basic_attack.get("mAttackDelayCastOffsetPercent", 0.0) * 0.5
				
			if 'mAttackTotalTime' in basic_attack and 'mAttackCastTime' in basic_attack:
				ba_windup = basic_attack['mAttackCastTime']/basic_attack['mAttackTotalTime']
			else:
				ba_windup = 0.3 + basic_attack.get('mAttackDelayCastOffsetPercent', 0.0)

		tags = set(['Unit_' + x.strip().replace('=', '_') for x in root.get("unitTagsString", "").split('|')])
		unit = {
			"name":				    name.lower(),
			"healthBarHeight":	    root.get("healthBarHeight", 100.0),
			"baseMoveSpeed":		root.get("baseMoveSpeed", 0.0),
			"attackRange":		    root.get("attackRange", 0.0),
			"attackSpeed":		    root.get("attackSpeed", 0.0), 
			"attackSpeedRatio":	    root.get("attackSpeedRatio", 0.0), 
			"acquisitionRange":	    root.get("acquisitionRange", 0.0),
			"selectionRadius":	    root.get("selectionRadius", 0.0),
			"pathingRadius":		root.get("pathfindingCollisionRadius", 0.0),
			"gameplayRadius":	    root.get("overrideGameplayCollisionRadius", 65.0),
			"basicAtkWindup":	    ba_windup,
			"basicAtkCastTime":	    ba_cast_time,
			"basicAtk":			    ba_name.lower(),
			"tags":				    list(tags)
		}
		units[unit["name"]] = unit
		
		# Read spells
		for key, val in props.items():
			if "mSpell" not in val:
				continue
			
			s = val["mSpell"]
			calcs = perform_spell_calculations(s)
			if len(calcs['data_vals']) > 0 or len(calcs['calcs']) > 0:
				spell_calculations[val['mScriptName']] = calcs
			
			if s:
				icon_name = os.path.basename(s.get("mImgIconName", [""])[0]).lower().replace(".dds", "")
				spell = {
					"name":	    	      os.path.basename(key).lower(),
					"icon":			      icon_name,
					"flags":			  s.get("mAffectsTypeFlags", 0),
					"castTime":		      s.get("mCastTime", 0.0),
					"castRange":		  s.get("castRangeDisplayOverride", s.get("castRange", [s.get("castConeDistance", 0.0)]))[0],
					"castRadius":		  s.get("castRadiusSecondary", s.get("castRadius", [0.0]))[0],
					"width":			  s.get("mLineWidth", 0.0),
					"height":			  0.0,
					"speed":			  s.get("missileSpeed", 0.0),
					"travelTime":		  0.0,
					"projectDestination": False
				}
				
				if spell["castTime"] == 0:
					if "delayCastOffsetPercent" in s:
						spell["castTime"] = 0.5 + 0.5 * s.get("delayCastOffsetPercent", 0.5)
					
				if spell['speed'] == 0.0:
					spell['speed'] = 10000.0
				
				if 'mCastRangeGrowthMax' in s:
					spell['castRange'] = s['mCastRangeGrowthMax'][4]
				
				missile = s.get("mMissileSpec", None)
				if missile:
					movcomp = missile.get("movementComponent", None)
					if movcomp:
						spell["speed"]	     	    = movcomp.get("mSpeed", spell["speed"])
						spell["height"]	     	    = movcomp.get("mOffsetInitialTargetHeight", 100.0)
						spell["projectDestination"] = movcomp.get("mProjectTargetToCastRange", False)
						spell["travelTime"]	        = movcomp.get("mTravelTime", 0.0)
						
				spells[spell["name"]] = spell
		
		
	print(f'Found {len(units)} units and {len(spells)} spells')
	
	with open("UnitData.json", 'w') as f:
		f.write(json.dumps(list(units.values()), indent=4))
		
	with open("SpellData.json", 'w') as f:
		f.write(json.dumps(list(spells.values()), indent=4))
		
	with open("SpellCalculations.json", 'w') as f:
		f.write(json.dumps(spell_calculations, indent=4))
	

def extract_skin_info(folder):
	result = {}
	for file in os.listdir(folder):
		d = None
		with open(os.path.join(folder, file), 'rb') as f:
			d = json.loads(f.read())
		
		skins = []
		for skininfo in d['skins']:
			chromas = []
			if 'chromas' in skininfo:
				for chroma in skininfo['chromas']:
					chromas.append({'id': chroma['id'] % 100, 'color': int(chroma['colors'][0][1:], 16)})
					
			skins.append({
				"name": skininfo["name"],
				"id": skininfo['id'] % 100,
				"chromas": chromas
		   }) 
		
		result[d['alias'].lower()] = skins
		
	with open('SkinInfo.json', 'w') as f:
		f.write(json.dumps(result, indent=4))
	
mode = sys.argv[1]
if mode == 'skininfo':
	extract_skin_info('champ_skin_infos')
elif mode == 'unitinfo':
	extract_unit_info('champ_unit_infos')