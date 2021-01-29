#pragma once
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

#include "GameObject.h"
#include "GameUnit.h"
#include "GameChampion.h"
#include "GameMissile.h"
#include "GameSpell.h"

#include "SpellInfo.h"
#include "ItemInfo.h"
#include "UnitInfo.h"

#include "PyExecutionContext.h"
#include "GameState.h"

using namespace boost::python;

BOOST_PYTHON_MODULE(valkyrie) {
	
	class_<UnitInfo>("UnitStatic")
		.def_readonly("hp_bar_height",      &UnitInfo::healthBarHeight)
		.def_readonly("movement_speed",     &UnitInfo::baseMovementSpeed)
		.def_readonly("base_atk_range",     &UnitInfo::baseAttackRange)
		.def_readonly("base_atk_speed",     &UnitInfo::baseAttackSpeed)
		.def_readonly("atk_speed_ratio",    &UnitInfo::attackSpeedRatio)

		.def_readonly("acquisition_radius", &UnitInfo::acquisitionRange)
		.def_readonly("selection_radius",   &UnitInfo::selectionRadius)
		.def_readonly("pathing_radius",     &UnitInfo::pathRadius)
		.def_readonly("gameplay_radius",    &UnitInfo::gameplayRadius)

		.def_readonly("basic_atk_speed",    &UnitInfo::basicAttackMissileSpeed)
		.def_readonly("basic_atk_windup",   &UnitInfo::basicAttackWindup)
		;

	class_<ItemInfo>("ItemStatic")
		.def_readonly("id",                &ItemInfo::id)
		.def_readonly("cost",              &ItemInfo::cost)
		.def_readonly("mov_speed",         &ItemInfo::movementSpeed)
		.def_readonly("health",            &ItemInfo::health)
		.def_readonly("crit",              &ItemInfo::crit)
		.def_readonly("ap",                &ItemInfo::abilityPower)
		.def_readonly("mana",              &ItemInfo::mana)
		.def_readonly("armour",            &ItemInfo::armour)
		.def_readonly("magic_resist",      &ItemInfo::magicResist)
		.def_readonly("phys_damage",       &ItemInfo::physicalDamage)
		.def_readonly("attack_speed",      &ItemInfo::attackSpeed)
		.def_readonly("life_steal",        &ItemInfo::lifeSteal)
		.def_readonly("hp_regen",          &ItemInfo::hpRegen)
		.def_readonly("mov_speed_percent", &ItemInfo::movementSpeedPercent)
		;

	class_<SpellInfo>("SpellStatic")
		.def_readonly("icon",            &SpellInfo::icon)
		.def_readonly("delay",           &SpellInfo::delay)
		.def_readonly("cast_range",      &SpellInfo::castRange)
		.def_readonly("cast_radius",     &SpellInfo::castRadius)
		.def_readonly("width",           &SpellInfo::width)
		.def_readonly("height",          &SpellInfo::height)
		.def_readonly("speed",           &SpellInfo::speed)
		.def_readonly("travel_time",     &SpellInfo::travelTime)
		;

	class_<GameObject>("Obj")
		.def_readonly("name",            &GameObject::name)
		.def_readonly("index",           &GameObject::index)
		.def_readonly("net_id",          &GameObject::networkId)
		.def_readonly("team",            &GameObject::team)
		.def_readonly("pos",             &GameObject::pos)
		.def_readonly("visible",         &GameObject::isVisible)
		.def_readonly("last_seen",       &GameObject::lastSeen)
		;

	class_<GameSpell>("Spell")
		.def_readonly("name",            &GameSpell::name)
		.def_readonly("lvl",             &GameSpell::lvl)
		.def_readonly("ready_at",        &GameSpell::readyAt)
		.def_readonly("value",           &GameSpell::value)
		;

	class_<GameUnit, bases<GameObject>>("Unit")
		.def_readonly("dead",            &GameUnit::isDead)
		.def_readonly("mana",            &GameUnit::mana)
		.def_readonly("health",          &GameUnit::health)
		.def_readonly("max_health",      &GameUnit::maxHealth)
		.def_readonly("armor",           &GameUnit::armor)
		.def_readonly("magic_res",       &GameUnit::magicRes)
		.def_readonly("base_atk",        &GameUnit::baseAtk)
		.def_readonly("bonus_atk",       &GameUnit::bonusAtk)
		.def_readonly("move_speed",      &GameUnit::moveSpeed)
		.def_readonly("lvl",             &GameUnit::lvl)
		.def_readonly("expiry",          &GameUnit::expiry)
		.def_readonly("crit",            &GameUnit::crit)
		.def_readonly("crit_multi",      &GameUnit::critMulti)
		.def_readonly("ap",              &GameUnit::abilityPower)
		.def_readonly("atk_speed_multi", &GameUnit::atkSpeedMulti)
		.def_readonly("atk_range",       &GameUnit::attackRange)
		;

	class_<GameMissile, bases<GameObject>>("Missile")
		.def_readonly("start_pos",       &GameMissile::startPos)
		.def_readonly("end_pos",         &GameMissile::endPos)
		.def_readonly("src_index",       &GameMissile::srcIndex)
		.def_readonly("dest_index",      &GameMissile::destIndex)
		;

	class_<GameChampion, bases<GameUnit>>("Champion")
		.def_readonly("spells",          &GameChampion::spells)
		.def_readonly("items",           &GameChampion::items)
		;

	class_<PyExecutionContext>("Context")
		.def("log",                      &PyExecutionContext::Log)
		;
}