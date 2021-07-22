'''
	Utility script that generates a JSON file with the game's item data. It expects as input a json file from riot's ddragon API
'''
import json, sys, urllib.request
from pprint import pprint

url = urllib.request.urlopen('https://ddragon.leagueoflegends.com/cdn/11.15.1/data/en_US/item.json')
data = url.read()

items = []
jdata = json.loads(data)
jitems = jdata["data"]

for id, jitem in jitems.items():
	
	jstats = jitem["stats"]
	items.append({
		"movementSpeed":        jstats.get("FlatMovementSpeedMod", 0.0),
		"health":               jstats.get("FlatHPPoolMod", 0.0),
		"crit":                 jstats.get("FlatCritChanceMod", 0.0),
		"abilityPower":         jstats.get("FlatMagicDamageMod", 0.0),
		"mana":                 jstats.get("FlatMPPoolMod", 0.0),
		"armour":               jstats.get("FlatArmorMod", 0.0),
		"magicResist":          jstats.get("FlatSpellBlockMod", 0.0),
		"physicalDamage":       jstats.get("FlatPhysicalDamageMod", 0.0),
		"attackSpeed":          jstats.get("PercentAttackSpeedMod", 0.0),
		"lifeSteal":            jstats.get("PercentLifeStealMod", 0.0),
		"hpRegen":              jstats.get("FlatHPRegenMod", 0.0),
		"movementSpeedPercent": jstats.get("PercentMovementSpeedMod", 0.0),
		"cost":                 jitem["gold"]["total"],
		"id":                   int(id)
	})

with open("ItemData.json", 'w') as f:
	f.write(json.dumps(items, indent=4))