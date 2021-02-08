import json
from valkyrie import *

class TargeterClosestToPlayer:
	def __str__(self):
		return "Closest To Player"
		
	def get_score(self, ctx, target):
		return target.pos.distance(ctx.player.pos)

class TargeterLowestHealth:
	def __str__(self):
		return "Lowest Health"
		
	def get_score(self, ctx, target):
		return target.health
		
class TargeterMonsterRarity:
	def __str__(self):
		return "Largest Jungle Monster"
	
	def get_score(self, ctx, target):
		if target.has_tags(Unit.MonsterEpic):
			return -1000
		elif target.has_tags(Unit.MonsterLarge):
			return -100
		else:
			return -10

TargetersChampion = [
	TargeterClosestToPlayer(),
	TargeterLowestHealth()
]
TargetersMonster = TargetersChampion + [TargeterMonsterRarity()]

class TargetSelector:
	
	targeters = [
		TargeterClosestToPlayer(),
		TargeterLowestHealth()
	]
	
	selected_targeter = 0
	
	def ui(self, ctx, label):
		ui = ctx.ui
		self.selected_targeter = ui.combo(label, self.targeters, self.selected_targeter)
		
	def get_target(self, ctx, targets, radius):
		
		best_target = None
		min_score   = 10000000
		for target in targets:
			if target.dead or not target.visible or not target.targetable:
				continue
				
			if target.pos.distance(ctx.player.pos) > radius:
				continue
				
			score = self.targeters[self.selected_targeter].get_score(ctx, target)
			if(score < min_score):
				min_score = score
				best_target = target
				
		return best_target
		
	def __str__(self):
		return json.dumps([self.selected_targeter])
		
	@classmethod
	def from_str(self, s):
		selector = TargetSelector()
		j = json.loads(s)
		selector.selected_targeter = j[0]
		
		return selector