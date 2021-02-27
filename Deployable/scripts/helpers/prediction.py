from valkyrie  import *
from .		   import items

def predict_minions_lasthit(ctx, minions, delay_percent = 0.0):
	'''
		Predicts the health of minions before the player basic attack hits, it returns a tuple (minion, new_health, hit_dmg) where:
			minion	 -> is the minion object
			new_health -> is the health of the minion after its been hit by the player
			hit_dmg	-> the hit dmg from the player
	'''
	
	player			 = ctx.player
	player_range	 = ctx.player.atk_range + ctx.player.static.gameplay_radius
	basic_atk_speed	 = player.static.basic_atk.speed
	basic_atk_delay	 = player.static.basic_atk_windup*(1.0 + delay_percent) / player.atk_speed
	result = []
	
	for enemy_minion in minions:
		if enemy_minion.dead or enemy_minion.ally_to(player):
			continue
		 
		hit_dmg		 		= items.get_onhit_physical(player, enemy_minion) + items.get_onhit_magical(player, enemy_minion)
		t_until_player_hits = basic_atk_delay + player.pos.distance(enemy_minion.pos) / basic_atk_speed
		enemy_minion_hp		= predict_minion_health(ctx, enemy_minion, minions, t_until_player_hits, delay_percent)
		
		if enemy_minion.pos.distance(player.pos) < player_range:
			result.append((enemy_minion, enemy_minion_hp, hit_dmg))
				
	return result

def predict_minion_health(ctx, enemy_minion, minions, t_future, delay_percent = 0.0):
	'''
		Predicts the minion health of `enemy_minion` after `t_future` seconds elapse.
		It accomplishes this by checking the basic attacks of minions that are enemy to `enemy_minion`.
	'''
	enemy_minion_hp = enemy_minion.health
	for ally_minion in minions:
		if ally_minion.dead or enemy_minion.ally_to(ally_minion):
			continue
		
		casting = ally_minion.curr_casting
		if casting and casting.dest_index == enemy_minion.index:
			t_until_ally_hits = (casting.cast_time - (ctx.time - casting.time_begin)) + ally_minion.pos.distance(enemy_minion.pos)/casting.static.speed
			t_until_ally_hits *= (1.0 - delay_percent)
			
			if t_until_ally_hits > 0.0 and t_until_ally_hits < t_future:
				enemy_minion_hp -= enemy_minion.base_atk
 
	return enemy_minion_hp