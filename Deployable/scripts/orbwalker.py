from valkyrie import *
from helpers.targeting import TargetSelector
from time import time
from enum import Enum

script_info = {
    'author': 'leryss',
    'description': 'none',
    'name': 'Orbwalker'
}

target_selector = TargetSelector()
max_atk_speed   = 2.2
key_kite        = 0
key_last_hit    = 0
key_lane_push   = 0
ping            = 60.0

class OrbwalkKite:
    def get_target(self, ctx):
        return target_selector.get_target(ctx.player, ctx.champs, ctx.player.atk_range)

class OrbwalkLastHit:
    def get_target(self, ctx):
        return None

class OrbwalkLanePush:
    def get_target(self, ctx):
        return None

kite_mode       = OrbwalkKite()
last_hit_mode   = OrbwalkLastHit()
lane_push_mode  = OrbwalkLanePush()

def valkyrie_menu(ctx):
    global target_selector, max_atk_speed, ping
    global key_kite, key_last_hit, key_lane_push
    ui = ctx.ui
    
    target_selector.ui(ctx)
    max_atk_speed  = ui.sliderfloat("Attack speed cap", max_atk_speed, 1.0, 3.0)
    ping           = ui.sliderfloat("Your average ping", ping, 1.0, 200.0)
    key_kite       = ui.keyselect("Key kite champions", key_kite)
    key_last_hit   = ui.keyselect("Key last hit minions", key_last_hit)
    key_lane_push  = ui.keyselect("Key lane push", key_lane_push)

def valkyrie_on_load(ctx):
    global target_selector, max_atk_speed, ping
    global key_kite, key_last_hit, key_lane_push
    cfg = ctx.cfg
    
    target_selector = TargetSelector.from_str(cfg.get_str("target", str(target_selector)))
    max_atk_speed   = cfg.get_float("max_atk_speed", max_atk_speed)
    ping            = cfg.get_float("ping", ping)
    key_kite        = cfg.get_int("key_kite", key_kite)
    key_last_hit    = cfg.get_int("key_last_hit", key_last_hit)
    key_lane_push   = cfg.get_int("key_lane_push", key_lane_push)
    
def valkyrie_on_save(ctx):
    cfg = ctx.cfg
    
    cfg.set_str("target", str(target_selector))
    cfg.set_float("max_atk_speed", max_atk_speed)
    cfg.set_float("ping", ping)
    cfg.set_int("key_kite", key_kite)
    cfg.set_int("key_last_hit", key_last_hit)
    cfg.set_int("key_lane_push", key_lane_push)

last_moved    = 0
last_attacked = 0

def valkyrie_exec(ctx):
    global last_moved, last_attacked
     
    mode = None
    if ctx.is_held(key_kite):
        mode = kite_mode
    elif ctx.is_held(key_last_hit):
        mode = last_hit_mode
    elif ctx.is_held(key_lane_push):
        mode = lane_push_mode
    else:
        return
    
    player        = ctx.player   
    atk_speed     = player.atk_speed
    b_windup_time = player.static.basic_atk_windup/player.static.base_atk_speed
    c_atk_time    = 1.0/atk_speed
    max_atk_time  = 1.0/max_atk_speed
    
    target = None
    now = time()
    dt = now - last_attacked - ping / 2000.0
    if dt > max(c_atk_time, max_atk_time):
        target = mode.get_target(ctx)
        if target:
            ctx.attack(target)
            last_attacked = now
            
    if not target and dt > b_windup_time and now - last_moved > 0.12:
        ctx.move()
        last_moved = now