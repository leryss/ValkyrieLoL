import json

class TargeterClosest:
    def __str__(self):
        return "Closest"
        
    def get_score(self, player, target, dist):
        return dist

class TargeterLowestHealth:
    def __str__(self):
        return "Lowest Health"
        
    def get_score(self, player, target, dist):
        return target.health

class TargetSelector:
    
    targeters = [
        TargeterClosest(),
        TargeterLowestHealth()
    ]
    
    selected_targeter = 0
    
    def ui(self, ctx):
        ui = ctx.ui
        self.selected_targeter = ui.combo("Target priority", self.targeters, self.selected_targeter)
        
    def get_target(self, player, targets, radius):
        
        best_target = None
        min_score   = 10000000
        for target in targets:
            if target.dead or not target.visible or not target.targetable or target.ally_to(player):
                continue
                
            dist = target.pos.distance(player.pos)
            if dist > radius:
                continue
                
            score = self.targeters[self.selected_targeter].get_score(player, target, dist)
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