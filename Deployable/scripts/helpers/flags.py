from collections import OrderedDict

class EvadeFlags:
	EvadeEndTime         = 0
	EvadePoint           = None
	CurrentEvadePriority = -1
	
	
class Orbwalker:
	Present = False
	Attacking = False
	
class KiteFlags:
	BasicAttackKiting = False
	SpellKiting       = False