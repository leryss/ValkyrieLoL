from valkyrie import *

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
	Poison  = 8

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
		Buff('RedPot',               'Item2003',                    BuffType.Potion),
		
		Buff('CassiopeiaQPoison',    'cassiopeiaqdebuff',           BuffType.Poison),
		Buff('CassiopeiaWPoison',    'cassiopeiawbuff',             BuffType.Poison),
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
	def has_buff_type(self, champ, buff_type):
		if type(champ) != ChampionObj:
			return False
			
		for buff in champ.buffs:
			b = Buffs.get(buff.name)
			if b.type == buff_type:
				return True
		return False
	
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
	
	SlotToStr = ['Q', 'W', 'E', 'R', 'D', 'F']
	
	@classmethod
	def to_str(self, slot):
		return self.SlotToStr[slot]

class SpellPredictor:
	def _ui(self, ctx, ui):
		pass
	
	def _predict(self, ctx, player, target, spell):
		return ctx.predict_cast_point(player, target, spell)

class RSpell:
	
	def __init__(self, slot, condition = None, predictor = SpellPredictor()):
		self.slot = slot
		self.condition = condition
		self.predictor = predictor
	
	def ui(self, ctx, ui):
		if not self.condition:
			return
		
		if ui.treenode('Trigger Condition'):
			ui.pushid(id(self.condition))
			self.condition.ui(ctx, ui)
			ui.separator()
			ui.popid()
			
			ui.treepop()
		ui.help('A trigger condition is logic that applies before casting the spell. For example a simple trigger would be "Target health trigger" that would check if the target HP is below a certain %, if the trigger is True then the spell is cast.')
		
	def check_condition(self, ctx, player, target, spell):
		if not self.condition:
			return True
			
		return self.condition.check(ctx, player, target, spell)
		
class SpellCondition:
	
	def __init__(self, enabled = True):
		self.enabled = enabled

	def check(self, ctx, player, target, spell):
		if self.enabled:
			return self._check(ctx, player, target, spell)
		return True

	def _check(self, ctx, player, target, spell):
		return True
		
	def _get_name(self):
		return 'Base Condition'
	
	def _get_help(self):
		return 'Condition help text'
	
	def ui(self, ctx, ui, depth = 0):
		
		self.enabled = ui.checkbox(self._get_name(), self.enabled)
		ui.help(self._get_help())
		if self.enabled:
			self._ui(ctx, ui)
		
	def _ui(self, ctx, ui):
		pass
		
class SpellRotation:
	'''
		Represents a rotation of spells
	'''
	
	def __init__(self, rotation_spells, mask = None):
		'''
			rotation_spells: must be an array of RSpell's
			mask: must be an array of booleans
		'''
		self.rotation_spells = rotation_spells
		if mask == None or len(rotation_spells) != len(mask):
			self.mask = [True for i in range(len(rotation_spells))]
		else:
			self.mask = mask
				
	def find_spell(self, ctx, target_selector):
		'''
			Gets the next castable spell in the rotation. Returns None if nothing found
		'''
		player = ctx.player
		spells = player.spells
		
		for i, rspell in enumerate(self.rotation_spells):
			if not self.mask[i]:
				continue
				
			spell = spells[rspell.slot]
			if not spell.static or not player.can_cast_spell(spell):
				continue
				
			target = target_selector.get_target(ctx, ctx.champs.enemy_to(player).near(player, spell.static.cast_range).get())
			if not target:
				continue
				
			if not rspell.check_condition(ctx, player, target, spell):
				continue
				
			return target, spell, rspell
		
		return None, None, None
		
	def get_spell(self, ctx, player, target_selector):
		if player.curr_casting and player.curr_casting.remaining > 0.0:
			return None, None

			
		target, spell, rspell = self.find_spell(ctx, target_selector)
		if not spell:
			return None, None
		
		point = rspell.predictor._predict(ctx, player, target, spell)
		if not point:
			return None, None
		
		return spell, point
		
	def cast(self, ctx, target_selector):
		player = ctx.player

		spell, point = self.get_spell(ctx, player, target_selector)
		if spell:
			ctx.cast_spell(spell, point)
	
	def ui(self, ctx, ui):
		for i in range(len(self.mask)):
			slot = self.rotation_spells[i].slot
			slot_str = Slot.to_str(slot)
			
			spell = ctx.player.spells[slot]
			ui.image(spell.static.icon if spell.static else 'None', Vec2(16, 16), Col.White)
			ui.sameline()
			if ui.beginmenu(slot_str + ' ' + spell.name):
				self.mask[i] = ui.checkbox('Use spell', self.mask[i])
				self.rotation_spells[i].ui(ctx, ui)
				ui.endmenu()
