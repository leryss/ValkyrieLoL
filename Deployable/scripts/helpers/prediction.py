from valkyrie  import *
from .         import items

def predict_minions_lasthit(ctx, minions):
    player              = ctx.player
    player_range        = ctx.player.atk_range + ctx.player.static.gameplay_radius
    basic_atk_speed     = player.static.basic_atk.speed
    basic_atk_delay     = player.static.basic_atk_windup / player.static.base_atk_speed
    
    result = []
    
    for enemy_minion in minions:
        if enemy_minion.dead or enemy_minion.ally_to(player):
            continue
         
        hit_dmg             = items.get_onhit_physical(player, enemy_minion) + items.get_onhit_magical(player, enemy_minion)
        t_until_player_hits = basic_atk_delay + ctx.ping / 2000.0 + player.pos.distance(enemy_minion.pos) / basic_atk_speed  #0.3087
        enemy_minion_hp     = predict_minion_health(ctx, enemy_minion, minions, t_until_player_hits)
        
        if enemy_minion.pos.distance(player.pos) < player_range:
            result.append((enemy_minion, enemy_minion_hp, hit_dmg))
                
    return result

def predict_minion_health(ctx, enemy_minion, minions, t_future):
    
    enemy_minion_hp = enemy_minion.health
    for ally_minion in minions:
        if ally_minion.dead or enemy_minion.ally_to(ally_minion):
            continue
        
        casting = ally_minion.curr_casting
        if casting and casting.dest_index == enemy_minion.index:
            t_until_ally_hits = casting.cast_time + ally_minion.pos.distance(enemy_minion.pos)/casting.static.speed - (ctx.time - casting.time_begin)
            
            if t_until_ally_hits > 0.0 and t_until_ally_hits < t_future:
                enemy_minion_hp -= enemy_minion.base_atk
 
    return enemy_minion_hp