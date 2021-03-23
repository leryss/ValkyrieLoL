from valkyrie import *
from helpers.targeting import TargetSelector, TargetSet
from helpers.spells import SpellRotation, RSpell, Slot, SpellCondition
import helpers.templates as HT
from helpers.damages import calculate_raw_spell_dmg

class ConditionKogMawUlt(SpellCondition):

	ranges_ult = [1300, 1550, 1800]
	
	def __init__(self, max_stacks, min_mana):
		self.enabled = True
		self.max_stacks = max_stacks
		self.min_mana = min_mana
		
	def _ui(self, ctx, ui):
		self.max_stacks = ui.sliderint('Max R stacks', self.max_stacks, 1, 9)
		self.min_mana   = ui.sliderfloat('Minimum mana for R', self.min_mana, 100, 1000.0)
		
	def _check(self, ctx, player, target, spell):
		
		if player.mana < self.min_mana:
			return False
			
		# Valkyrie engine doesnt support multiple ranges on the same skill so we have to handle it ourselves
		if player.pos.distance(target.pos) > self.ranges_ult[spell.lvl - 1]:
			return False
		
		num_stacks = player.num_buff_stacks('kogmawlivingartillerycost')
		if num_stacks*spell.mana > player.mana:
			return False
			
		if num_stacks > self.max_stacks:
			return False
			
		return True
		
	def _get_name(self):
		return 'Kogmaw ult management'
		
	def _get_help(self):
		return 'Condition for kogmaw ult management'

kogmaw = HT.ChampionScript(
	passive_trigger   = HT.Enabler.default(),
	combat_distance   = 1800,
	passive_distance  = 1800,
	
	combat_rotation = SpellRotation([
		RSpell(Slot.Q, HT.MixedConditions([
			HT.ConditionAttribute(HT.Attributes.Armor, 100.0, 300.0),
			HT.ConditionAttribute(HT.Attributes.MagicResist, 100.0, 300.0)
		], HT.MixedConditions.Any)), 
		RSpell(Slot.W), 
		RSpell(Slot.E, HT.ConditionDistanceToTarget(700, 3000)), 
		RSpell(Slot.R, ConditionKogMawUlt(9, 200)),
	]),

	passive_rotation = SpellRotation([
		RSpell(Slot.R, ConditionKogMawUlt(1, 200))
	])
)

def valkyrie_menu(ctx) :
	ui = ctx.ui					 

	kogmaw.ui(ctx)
	
def valkyrie_on_load(ctx) :	 
	global kogmaw
	cfg = ctx.cfg				 
	
	kogmaw = HT.ChampionScript.from_str(cfg.get_str("kogmaw", str(kogmaw)))
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg				 
	cfg.set_str('kogmaw', str(kogmaw))
	
def valkyrie_exec(ctx) :	     
	if ctx.player.dead:
		return
	
	kogmaw.exec(ctx)
