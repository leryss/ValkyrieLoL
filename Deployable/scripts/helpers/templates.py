import json
from valkyrie import *
from .spells import SpellKiter
from .targeting import TargetSelector
from .flags import Orbwalker
from .inputs import KeyInput

class Enabler:

	enable_type = ['Always On', 'Key Input']

	def __init__(self, always_on, keyinput):
		self.keyinput  = keyinput
		self.selected  = 0 if always_on else 1
		
	def ui(self, ui):
		self.selected = ui.sliderenum('Activation Type', self.enable_type[self.selected], self.selected, 1)
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

class ChampionScript:
	'''
		Template for a starter champion script. 
		Currently supports combat and harras using spell rotations.
		
		Planned features: combos, last hit, farm/push lane
	'''
	
	def __init__(self, harras_on):
		self.harras_on = harras_on
		
	def setup_harras(self, target_distance, rota):
		'''
			Sets up the harras mode. target_distance is the minimum distance to the target champion for harras
		'''
		self.harras_kiter = SpellKiter(rota, target_distance)
		
	def setup_combat(self, target_distance, rota):
		'''
			Sets up the combat mode. target_distance is the minimum distance to the target champion for combat
		'''
		self.combat_kiter = SpellKiter(rota, target_distance)
		
	def ui(self, ctx):
		ui = ctx.ui
		
		self.harras_on  = ui.checkbox('Auto harras', self.harras_on)
		ui.separator()
		
	def exec(self, ctx):
		player = ctx.player
		if Orbwalker.CurrentMode:
			if Orbwalker.CurrentMode == Orbwalker.ModeKite:
				ctx.pill('Turbo', Col.Green, Col.Black)
				self.combat_kiter.kite(ctx, Orbwalker.CurrentMode.get_target(ctx))
				return
			
		if self.harras_on and Orbwalker.ModeKite:
			ctx.pill('Harras', Col.Purple, Col.Black)
			spell, point = self.harras_kiter.get_spell(ctx, player, Orbwalker.ModeKite.get_target(ctx))
			if spell:
				ctx.cast_spell(spell, point)
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		return ChampionScript(
			harras_on = j['harras_on']
		)
		
	def __str__(self):
		return json.dumps({
			'harras_on': self.harras_on,
		})