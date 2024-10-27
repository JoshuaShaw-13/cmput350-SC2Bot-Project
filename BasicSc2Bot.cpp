#include "BasicSc2Bot.h"
#include "GameManager.h"
#include <sc2api/sc2_interfaces.h>

using namespace sc2;

GameManager state;

void BasicSc2Bot::OnGameStart() { return; }

void BasicSc2Bot::OnStep() { std::cout << "Hello, World!" << std::endl; }

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
  state.idleUnits.push_back(unit);
  switch (unit->unit_type.ToType()) {
  case UNIT_TYPEID::ZERG_HATCHERY: {
    break;
  }
  case UNIT_TYPEID::ZERG_LAIR: {
    break;
  }
  case UNIT_TYPEID::ZERG_ROACHWARREN: {
    break;
  }
  case UNIT_TYPEID::ZERG_DRONE: {
    const Unit *mineral_target = FindNearestMineralPatch(unit->pos);
    if (!mineral_target) {
      break;
    }
    Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
    break;
  }
  case UNIT_TYPEID::ZERG_OVERLORD: {
    if (state.scouts.size() < 1) {
      Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                             state.scouting_location);
      state.scouts.push_back(unit);
    } else {
      Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                             state.overlord_rally_point);
    }
    break;
  }
  case UNIT_TYPEID::ZERG_QUEEN: {
    Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA);
    break;
  }
  case UNIT_TYPEID::ZERG_ZERGLING: {
    Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, state.rally_point);
    break;
  }
  case UNIT_TYPEID::ZERG_ROACH: {
    Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, state.rally_point);
    break;
  }
  case UNIT_TYPEID::ZERG_LARVA: {
    break;
  }
  default: {
    break;
  }
  }
}

const Unit *BasicSc2Bot::FindNearestMineralPatch(const Point2D &start) {
  Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
  float distance = std::numeric_limits<float>::max();
  const Unit *target = nullptr;
  for (const auto &u : units) {
    if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
      float d = DistanceSquared2D(u->pos, start);
      if (d < distance) {
        distance = d;
        target = u;
      }
    }
  }
  return target;
}