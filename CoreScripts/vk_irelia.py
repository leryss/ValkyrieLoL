from valkyrie import *
from helpers.targeting import TargetSelector, TargetSet
from helpers.spells import SpellRotation, RSpell, Slot, SpellCondition
import helpers.templates as HT
from helpers.damages import calculate_raw_spell_dmg

class ConditionIreliaMark(SpellCondition):

	def __init__(self, marked):
		self.enabled = True
		self.marked = marked
		
	def _ui(self, ctx, ui):
		self.marked = ui.checkbox('Must have irelia mark', self.marked)
		
	def _check(self, ctx, player, target, spell):
		count = target.num_buff_stacks('ireliamark')
		if self.marked:
			return count > 0
		return count == 0
		
	def _get_name(self):
		return 'Irelia mark management'
		
	def _get_help(self):
		return 'Condition for irelia mark produced by E, R'
		
class ConditionIreliaQ(ConditionIreliaMark):

	cd_q = [ 11 , 10 , 9 , 8 , 7]
	
	def __init__(self, marked, smartq):
		ConditionIreliaMark.__init__(self, marked)
		self.smartq = smartq
		
	def _check(self, ctx, player, target, spell):
		result = ConditionIreliaMark._check(self, ctx, player, target, spell)
		if result or not self.smartq:
			return result
		
		cd_q = self.cd_q[spell.lvl - 1]
		cd_q -= cd_q*player.cdr
		
		e = player.spells[Slot.E]
		e_cd = e.cd
		r = player.spells[Slot.R]
		r_cd = r.cd
		if player.can_cast_spell(e) or player.can_cast_spell(r) or self.exists_missile(ctx, 'ireliae') or self.exists_missile(ctx, 'ireliar'):
			return False
		
		cd_min = min(e_cd if e.lvl > 0 else 1000, r_cd if r.lvl > 0 else 1000)
		return cd_q < cd_min
		
	def exists_missile(self, ctx, missile_name):
		for m in ctx.missiles.get():
			if m.name.startswith(missile_name):
				return True
		return False
	
	def _ui(self, ctx, ui):
		ConditionIreliaMark._ui(self, ctx, ui)
		self.smartq = ui.checkbox('Maximize Q casts', self.smartq)
	
	def _get_name(self):
		return 'Irelia smart Q'
		
	def _get_help(self):
		return 'Conditions specific to irelias Q'
	
class IreliaEPredictor:
	def _predict(self, ctx, player, target, spell):
		if spell.name == 'ireliae':
			return target.pos + (target.dir * -200)
		else:
			return ctx.predict_cast_point(player, target, spell)


irelia = HT.ChampionScript(
	passive_trigger   = HT.Enabler.default(),
	combat_distance   = 1000,
	passive_distance  = 1000,
	
	combat_rotation = SpellRotation([
		RSpell(Slot.R, ConditionIreliaMark(False)),
		RSpell(
			Slot.Q, 
			condition = HT.MixedConditions([ConditionIreliaQ(True, True), HT.ConditionKillable()], HT.MixedConditions.Any)
		),
		RSpell(Slot.E, ConditionIreliaMark(False), predictor = IreliaEPredictor()),
		RSpell(Slot.W),
	]),

	passive_rotation = SpellRotation([	
	])
)

def valkyrie_menu(ctx) :
	ui = ctx.ui					 

	irelia.ui(ctx)
	
def valkyrie_on_load(ctx) :	 
	global irelia
	cfg = ctx.cfg				 
	
	irelia = HT.ChampionScript.from_str(cfg.get_str("irelia", str(irelia)))
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	cfg.set_str('irelia', str(irelia))
	
def valkyrie_exec(ctx) :	     
	if ctx.player.dead:
		return
	
	irelia.exec(ctx)
