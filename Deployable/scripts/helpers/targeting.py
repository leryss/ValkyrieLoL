import json
from valkyrie import *
from enum import Enum

class BaseTargeter:
	def get_score(self, ctx, target):
		return 0
		
	def ui(self, ctx, ui):
		pass

class TargeterClosestToPlayer(BaseTargeter):
	def __str__(self):
		return "Closest To Player"
		
	def get_score(self, ctx, target):
		return target.pos.distance(ctx.player.pos)

class TargeterLowestHealth(BaseTargeter):
	def __str__(self):
		return "Lowest Health"
		
	def get_score(self, ctx, target):
		return target.health
		
class TargeterClosestToMouse(BaseTargeter):
	def __str__(self):
		return "Closest To Mouse"
		
	def get_score(self, ctx, target):
		return ctx.cursor_pos.distance(ctx.w2s(target.pos))
		
class TargeterChampionPriority(BaseTargeter):
	
	def __init__(self):
		self.priorities = {}
		self.initialized = False
		
	def __str__(self):
		return "Champion Priority"
		
	def get_score(self, ctx, target):
		if not self.initialized:
			self.init(ctx)
		return -self.priorities.get(target.name, 0)
		
	def ui(self, ctx, ui):
		if not self.initialized:
			self.init(ctx)
		for name, old_prio in self.priorities.items():
			new_prio = ui.sliderint(name, old_prio, 0, 5)
			self.priorities[name] = new_prio
	
	def init(self, ctx):
		for champ in ctx.champs.enemy_to(ctx.player).get():
			self.priorities[champ.name] = 3
		self.initialized = True
	
class TargeterMonsterLargest(BaseTargeter):
	def __str__(self):
		return "Largest Jungle Monster"
	
	def get_score(self, ctx, target):
		if target.has_tags(Unit.MonsterEpic):
			return -1000
		elif target.has_tags(Unit.MonsterLarge):
			return -100
		else:
			return -10
			
class TargeterMonsterSmallest(BaseTargeter):
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
			TargeterLowestHealth(),
			TargeterClosestToMouse(),
			TargeterChampionPriority()
		],
		
		TargetSet.Monster: [
			TargeterClosestToPlayer(),
			TargeterLowestHealth(),
			TargeterMonsterLargest(),
			TargeterMonsterSmallest(),
			TargeterClosestToMouse()
		]
	}
	
	def __init__(self, selected = 0, target_set = TargetSet.Champion):
		self.target_set        = target_set
		self.targeters         = self.target_sets[target_set]
		self.selected_targeter = selected
	
	def ui(self, label, ctx, ui):
		ui.pushid(id(self))
		self.selected_targeter = ui.combo(label, self.targeters, self.selected_targeter)
		self.targeters[self.selected_targeter].ui(ctx, ui)
		ui.popid()

	def get_target(self, ctx, targets):
		
		best_target = None
		min_score   = 10000000
		for target in targets:
			
			score = self.targeters[self.selected_targeter].get_score(ctx, target)
			if hasattr(target, 'is_clone') and target.is_clone:
				score += 1000
				
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