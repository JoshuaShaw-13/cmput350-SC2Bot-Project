#include "BasicSc2Bot.h"
#include "GameState.h"
#include <sc2api/sc2_interfaces.h>

using namespace sc2;

GameState state;

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
    // if less than 3 scouting, go scout
    // if not go to back of base
    break;
  }
  case UNIT_TYPEID::ZERG_QUEEN: {
    Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA);
    break;
  }
  case UNIT_TYPEID::ZERG_ZERGLING: {
    // rally
    break;
  }
  case UNIT_TYPEID::ZERG_ROACH: {
    // rally
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