import os, json
from enum import Enum
from time import time
from .flags import Orbwalker, EvadeFlags

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
		'''
			Check if champion has a buff.
		'''
		buff_obj = Buffs.AllBuffsDict.get(buff_name, None)
		if buff_obj == None:
			return False
			
		return champ.has_buff(buff_obj.name)
	
	@classmethod
	def get(self, buff_name):
		'''
			Gets the buff object by name of the buff
		'''
		return Buffs.AllBuffsDict.get(buff_name, Buffs.UnknownBuff)

class Slot:
	''' 
		Spell slot indices. Usage: ChampionObj.spells[Slot.Q] 
	'''
	Q = 0
	W = 1
	E = 2
	R = 3
	D = 4
	F = 5

class RSpell:
	''' 
		Rotation Spell info. 
		condition argument must be a function with arguments (ctx=Context, player=ChampionObj, target=UnitObj, spell=GameSpell).
		condition function must return a boolean if spell should be cast
	'''
	def __init__(self, slot, condition = None):
		self.slot = slot
		self.condition = condition
		
class SpellRotation:
	'''
		Represents a rotation of spells
	'''
	
	def __init__(self, rotation_spells):
		'''
			rotation_spells must be an array of RSpell's
		'''
		self.rotation_spells = rotation_spells
	
	def get_spell(self, ctx, target):
		'''
			Gets the next castable spell in the rotation. Returns None if nothing found
		'''
		player = ctx.player
		spells = player.spells
		
		for rspell in self.rotation_spells:
			spell = spells[rspell.slot]
			if not spell.static:
				continue
			if rspell.condition and not rspell.condition(ctx, player, target, spell):
				continue
				
			if player.can_cast_spell(spell) and target.pos.distance(player.pos) < spell.static.cast_range:
				return spell
		
		return None
		
class SpellKiter:
	'''
		Similar to orbwalker but instead of attacks it casts spells from a SpellRotation
	'''
	
	cooldown_move = 0.08
	
	def __init__(self, selector, rotation, target_distance):
		self.selector = selector
		self.rotation = rotation
		self.target_distance = target_distance
		self.last_moved = 0
		
	def kite(self, ctx):
		player = ctx.player
		
		if Orbwalker.Attacking:
			return
		
		now = time()
		if now < EvadeFlags.EvadeEndTime:
			return
		
		spell, point = self.get_spell(ctx, player)
		if spell:
			ctx.cast_spell(spell, point)
		else:
			if now - self.last_moved > self.cooldown_move:
				self.last_moved = now
				ctx.move()
		
	def get_spell(self, ctx, player):
		'''
			Gets next spell to cast from the rotation along with the cast point
		'''
		if player.curr_casting and player.curr_casting.remaining > 0.0:
			return None, None
		
		target = self.selector.get_target(ctx, ctx.champs.enemy_to(player).targetable().near(player, self.target_distance).get())
		if not target:
			return None, None
			
		spell = self.rotation.get_spell(ctx, target)
		if not spell:
			return None, None
		
		point = ctx.predict_cast_point(player, target, spell)
		if not point:
			return None, None
		
		return spell, point
