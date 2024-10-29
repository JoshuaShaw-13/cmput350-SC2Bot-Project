#include "BasicSc2Bot.h"
#include "GameManager.h"
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_typeenums.h>

using namespace sc2;

GameManager state;

void BasicSc2Bot::OnGameStart() { 
    const ObservationInterface* observation = Observation();
    scout_locations = observation->GetGameInfo().enemy_start_locations;

    // Build drones if not at the given army cap for next item 
        build_order.push(BuildOrderItem(13, UNIT_TYPEID::ZERG_OVERLORD));      // At 13 cap, build an Overlord
        build_order.push(BuildOrderItem(16, UNIT_TYPEID::ZERG_HATCHERY));      // At 16 cap, build a Hatchery
        build_order.push(BuildOrderItem(18, UNIT_TYPEID::ZERG_EXTRACTOR));     // At 18 cap, build an Extractor
        build_order.push(BuildOrderItem(17, UNIT_TYPEID::ZERG_SPAWNINGPOOL));  // At 17 cap, build a Spawning Pool
        build_order.push(BuildOrderItem(20, UNIT_TYPEID::ZERG_OVERLORD));      // At 20 cap, build an Overlord
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN));          // Build 2 Queens
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN));
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));       // Build 4 Zerglings
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
        build_order.push(BuildOrderItem(32, UNIT_TYPEID::ZERG_OVERLORD));      // At 32 cap, build an Overlord
        build_order.push(BuildOrderItem(0, ABILITY_ID::MORPH_LAIR));           // Upgrade Hatchery to Lair
        build_order.push(BuildOrderItem(36, UNIT_TYPEID::ZERG_OVERLORD));      // At 36 cap, build an Overlord
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ROACHWARREN));    // Build a Roach Warren
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR));      // Build 2 more Extractors
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR));
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER));   // Build 2 Spore Crawlers
        build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER));

    // Send the first Overlord to scout one of the scout_locations and remove it from further scouting consideration
    Units overlords = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
    if (!overlords.empty() && !scout_locations.empty()) {
        const Unit* first_overlord = overlords.front();
        Actions()->UnitCommand(first_overlord, ABILITY_ID::SMART, scout_locations.front());
        scout_locations.erase(scout_locations.begin());
    } }

void BasicSc2Bot::OnStep() { std::cout << "Hello, World!" << std::endl; }

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
  state.idleUnits.push_back(unit);
  switch (unit->unit_type.ToType()) {
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
