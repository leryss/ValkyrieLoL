import json
from valkyrie import *
from .spells import SpellKiter
from .targeting import TargetSelector

class ChampionScript:
	'''
		Template for a starter champion script. 
		Currently supports combat, harras using spell rotations with target selection.
		
		Planned features: combos, farm/push lane
	'''
	
	def __init__(self, target_selector, harras_on, combat_key):
		self.target_selector = target_selector
		
		self.harras_on = harras_on
		self.combat_key = combat_key
		
	def setup_harras(self, target_distance, rota):
		'''
			Sets up the harras mode. target_distance is the minimum distance to the target champion for harras
		'''
		self.harras_kiter = SpellKiter(self.target_selector, rota, target_distance)
		
	def setup_combat(self, target_distance, rota):
		'''
			Sets up the combat mode. target_distance is the minimum distance to the target champion for combat
		'''
		self.combat_kiter = SpellKiter(self.target_selector, rota, target_distance)
		
	def in_combat(self, ctx):
		'''
			Checks if combat mode is enabled
		'''
		return ctx.is_held(self.combat_key)
		
	def ui(self, ctx):
		ui = ctx.ui
		
		self.target_selector.ui('Target selector', ctx, ui)
		self.combat_key = ui.keyselect('Key combat', self.combat_key)
		self.harras_on  = ui.checkbox('Auto harras', self.harras_on)
		
	def exec(self, ctx):
		player = ctx.player
		
		if ctx.is_held(self.combat_key):
			ctx.pill('Combat', Col.Green, Col.Black)
			self.combat_kiter.kite(ctx)
			
		elif self.harras_on:
			ctx.pill('Harras', Col.Purple, Col.Black)
			spell, point = self.harras_kiter.get_spell(ctx, player)
			if spell:
				ctx.cast_spell(spell, point)
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		return ChampionScript(
			target_selector = TargetSelector.from_str(j['target_selector']),
			combat_key = j['combat_key'],
			harras_on = j['harras_on']
		)
		
	def __str__(self):
		return json.dumps({
			'target_selector': str(self.target_selector),
			'harras_on': self.harras_on,
			'combat_key': self.combat_key
		})