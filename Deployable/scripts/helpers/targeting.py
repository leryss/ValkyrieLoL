import json
from valkyrie import *
from enum import Enum

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
		
class TargeterMonsterLargest:
	def __str__(self):
		return "Largest Jungle Monster"
	
	def get_score(self, ctx, target):
		if target.has_tags(Unit.MonsterEpic):
			return -1000
		elif target.has_tags(Unit.MonsterLarge):
			return -100
		else:
			return -10
			
class TargeterMonsterSmallest:
	def __str__(self):
		return "Smallest Jungle Monster"
	
	def get_score(self, ctx, target):
		if target.has_tags(Unit.MonsterEpic):
			return -10
		elif target.has_tags(Unit.MonsterLarge):
			return -100
		else:
			return -1000

class TargetSet:
	Champion = 0
	Monster  = 1
	
class TargetSelector:
	
	target_sets = {
		TargetSet.Champion: [
			TargeterClosestToPlayer(),
			TargeterLowestHealth()
		],
		
		TargetSet.Monster: [
			TargeterClosestToPlayer(),
			TargeterLowestHealth(),
			TargeterMonsterLargest(),
			TargeterMonsterSmallest()
		]
	}
	
	def __init__(self, selected = 0, target_set = TargetSet.Champion):
		self.target_set        = target_set
		self.targeters         = self.target_sets[target_set]
		self.selected_targeter = selected
	
	def ui(self, label, ui):
		ui.pushid(id(self))
		self.selected_targeter = ui.combo(label, self.targeters, self.selected_targeter)
		ui.popid()

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
		return json.dumps([self.selected_targeter, self.target_set])
		
	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return TargetSelector(*j)