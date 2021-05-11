from valkyrie import *	
from importlib import import_module
import inspect, re
import pdoc

def add(ctx, cls):

	bases = {
		'MissileObj'  : 'Obj',
		'UnitObj'     : 'Obj',
		'JungleMobObj': 'UnitObj',
		'MinionObj'   : 'UnitObj',
		'TurretObj'   : 'UnitObj',
		'ChampionObj' : 'UnitObj',
	}

	if cls == None:
		return ''										
	
	result = ''	
	if inspect.isclass(cls):

		base = bases.get(cls.__name__, None)
		if base:
			result += f'class {cls.__name__}({base}):\n'
		else:
			result += f'class {cls.__name__}:\n'
		result += f"\t'''{cls.__doc__}'''\n\n"
		for name, val in cls.__dict__.items():
			
			if name.startswith('__'):
				continue
				
			t = type(val)
			str_t = str(t)
			
			# Handle enums
			if name == 'names':
				for n, v in zip(cls.__dict__['names'].keys(), cls.__dict__['values'].keys()):
					result += f'\t{n} = {v}\n'
				result += '\n'
			
			# Handle function
			elif 'function' in str_t:
				doc = val.__doc__
				if not doc:
					continue
					
				_, _, doc, _ = doc.split('\n', 3)
				find = re.findall('\[(.+)\]', doc)
				if len(find) == 0:
					continue
				
				proto = find[0]
				doc = re.sub('\[.+\]', '', doc)
				result += f'\tdef {proto}:\n'
				result += f"\t\t'''{doc}'''\n"
				result += f'\t\tpass\n\n'
			
			# Handle others
			else:
				doc = val.__doc__
				if not doc:
					continue
					
				find = re.findall('\[(.+)\]', doc)
				doc = re.sub('\[.+\]', '', doc)
				if len(find) == 0:
					continue
				
				prop_type = find[0]
				result += "\t@property\n"
				result += f"\tdef {name}(self) -> {prop_type}:\n"
				result += f"\t\t'''{doc}'''\n"
				result += f"\t\tpass\n\n"
	
	return result

def valkyrie_menu(ctx: Context):
	ui = ctx.ui
	
def valkyrie_on_load(ctx: Context) :
	objs = globals()
	result = '### ---> THIS IS AN AUTOMATICALLY GENERATED FILE. IT IS USED FOR AUTOCOMPLETION IN YOUR FAVORITE IDE <--- ###\n'
	result += 'from __future__ import annotations\n\n'
	for t, val in objs.items():
		result += add(ctx, val)

	with open('valkyrie.py', 'w') as f:
		f.write(result)
	
def valkyrie_on_save(ctx: Context) :	 
	cfg = ctx.cfg				 

def valkyrie_exec(ctx: Context) :
	pass
		
	
	
	
	
	
	
	
	
	
