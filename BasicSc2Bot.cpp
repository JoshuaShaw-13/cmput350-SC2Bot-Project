#include "BasicSc2Bot.h"
#include "GameManager.h"
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_unit_filters.h>


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

void BasicSc2Bot::OnStep() {
    const ObservationInterface* observation = Observation();
    int current_supply = observation->GetFoodUsed();
    // Change it to build at cap for example build drones till 13

    // build next structure/unit
    if(!build_order.isEmpty()){
        auto buildItem = build_order.peek();
        int count;
        if(current_supply >= buildItem.supply){
            if(tryBuild(buildItem)){
                build_order.pop();
            }
        }
        else{
            const Unit* larva = nullptr;
            larva = findIdleLarva();
            if(larva){
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
            }
        }
    }
   

    
    // build overseers when we are nearing troop capacity
    //buildOverseers();

    //check if we have enough resources to expand to a new base
    // this would be in the manager
    

    // Attack once we have an optimal army build
    // use the attack base queue
    if(isArmyReady()){
        if(!enemy_bases.isEmpty()){
            launchAttack(enemy_bases.peek()->loc);
        }
    }

    return; 
    }

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

const Unit* BasicSc2Bot::findIdleLarva(){
    for(const auto& unit: Observation()->GetUnits(Unit::Alliance::Self)){
        if(unit->unit_type == UNIT_TYPEID::ZERG_LARVA && unit->orders.empty()){
            return unit;
        }
    }
    return nullptr;
}
const Unit* BasicSc2Bot::findIdleDrone(){
    for(const auto& unit: Observation()->GetUnits(Unit::Alliance::Self)){
        if(unit->unit_type == UNIT_TYPEID::ZERG_DRONE && unit->orders.empty()){
            return unit;
        }
    }
    return nullptr;
}
// Eventually will use manager
// For now it checks whether its a unit, strucure, or upgrade
bool BasicSc2Bot::tryBuild(struct BuildOrderItem buildItem){
    const ObservationInterface* observation = Observation();
    // if its a unit build it if we can afford
    if(buildItem.is_unit){
        // check whether its a unit or a structure
        const Unit* larva = nullptr;
        switch(buildItem.unit_type){
            case UNIT_TYPEID::ZERG_DRONE:
                larva = findIdleLarva();
                if(larva){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_ROACH:
                larva = findIdleLarva();
                if(larva && observation->GetMinerals() >= 75 && observation->GetVespene() >= 25){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ROACH);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_ZERGLING:
                larva = findIdleLarva();
                if(larva && observation->GetMinerals() >= 50){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ZERGLING);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_QUEEN:
                for(const auto& hatchery: observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))){
                    if(hatchery->orders.empty() && observation->GetMinerals() >= 150){
                        Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_QUEEN);
                        return true;
                    }
                }
                break;
            
            default:
                const Unit* drone = findIdleDrone();
                if(drone){
                    float rx = GetRandomScalar();
                    float ry = GetRandomScalar();
                    Point2D build_position = Point2D(drone->pos.x + rx * 15.0f, drone->pos.y + ry * 15.0f);

                    Actions()->UnitCommand(drone, buildItem.ability, build_position);
                    return true;
                }
        }
        return false;
    }
    // must be an upgrade
    if(buildItem.ability != ABILITY_ID::INVALID){
        Units structures = observation->GetUnits(Unit::Alliance::Self, IsUnit(buildItem.unit_type));
        if(!structures.empty()){
            Actions()->UnitCommand(structures[0], buildItem.ability);
            return true;
        }
    }
    return false;
}


bool BasicSc2Bot::isArmyReady() {
    int roach_count;
    int zergling_count;
    const int optRoach = 10;
    const int optZergling = 20;
    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
            roach_count++;
        } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING) {
            zergling_count++;
        }
    }
    return roach_count >= optRoach && zergling_count >= optZergling;
}
void BasicSc2Bot::launchAttack(const Point2D& target){
    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING || unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
        }
    }
}
// void BuildOverseersIfNeeded(const ObservationInterface* observation) {
//     if (observation->GetFoodUsed() >= observation->GetFoodCap() - 2){
//         if (builder && observation->GetMinerals() >= 100) {
//             Actions()->UnitCommand(builder, ABILITY_ID::TRAIN_OVERSEER);
//         }
//     }
// }

