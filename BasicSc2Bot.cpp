#include "BasicSc2Bot.h"

void BasicSc2Bot::OnGameStart() { 
    const ObservationInterface* observation = Observation();
    scout_locations = observation->GetGameInfo().enemy_start_locations;
    enemy_bases.clear();
    // Build order queue: list of BuildOrderItems
    build_order_index = 0;
    // Build drones if not at the given army cap for next item 
    build_order = {
        BuildOrderItem(13, UNIT_TYPEID::ZERG_OVERLORD),      // At 13 cap, build an Overlord
        BuildOrderItem(16, UNIT_TYPEID::ZERG_HATCHERY),      // At 16 cap, build a Hatchery
        BuildOrderItem(18, UNIT_TYPEID::ZERG_EXTRACTOR),     // At 18 cap, build an Extractor
        BuildOrderItem(17, UNIT_TYPEID::ZERG_SPAWNINGPOOL),  // At 17 cap, build a Spawning Pool
        BuildOrderItem(20, UNIT_TYPEID::ZERG_OVERLORD),      // At 20 cap, build an Overlord
        BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN),          // Build 2 Queens
        BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN),
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING),       // Build 4 Zerglings
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING),
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING),
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING),
        BuildOrderItem(32, UNIT_TYPEID::ZERG_OVERLORD),      // At 32 cap, build an Overlord
        BuildOrderItem(0, ABILITY_ID::MORPH_LAIR),           // Upgrade Hatchery to Lair
        BuildOrderItem(36, UNIT_TYPEID::ZERG_OVERLORD),      // At 36 cap, build an Overlord
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ROACHWARREN),    // Build a Roach Warren
        BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR),      // Build 2 more Extractors
        BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR),
        BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER),   // Build 2 Spore Crawlers
        BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER),
        
    };

    // Assign all initial drones to gather minerals (Do they do this automatically?)
    Units drones = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    for (const auto& drone : drones) {
        if (drone->orders.empty()) {
            const Unit* closest_mineral = FindNearestMineralPatch(drone->pos);
            if (closest_mineral) {
                Actions()->UnitCommand(drone, ABILITY_ID::SMART, closest_mineral);
            }
        }
    }

    // Send the first Overlord to scout one of the scout_locations and remove it from further scouting consideration
    Units overlords = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
    if (!overlords.empty() && !scout_locations.empty()) {
        const Unit* first_overlord = overlords.front();
        Actions()->UnitCommand(first_overlord, ABILITY_ID::MOVE, scout_locations.front());
        scout_locations.erase(scout_locations.begin());
    } }

void BasicSc2Bot::OnStep() { 

    
    
}
