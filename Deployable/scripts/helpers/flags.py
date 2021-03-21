from collections import OrderedDict

class EvadeFlags:
	EvadeEndTime         = 0
	EvadePoint           = None
	CurrentEvadePriority = -1
	
class Orbwalker:
	Present      = False
	Attacking    = False
	
	CurrentMode      = None
	SelectorChampion = None
	SelectorMonster  = None
	
	ModeKite     = None
	ModeLastHit  = None
	ModeLanePush = None
	
class KiteFlags:
	BasicAttackKiting = False
	SpellKiting       = False