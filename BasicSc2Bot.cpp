#include "BasicSc2Bot.h"
#include "GameManager.h"
#include "cpp-sc2/include/sc2api/sc2_common.h"
#include "cpp-sc2/include/sc2api/sc2_gametypes.h"
#include "cpp-sc2/include/sc2api/sc2_typeenums.h"
#include <cmath>
#include <iostream>
#include <ostream>
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_unit_filters.h>
#include <string>
#include <utility>
#define M_PI 3.14

using namespace sc2;

GameManager state;

void BasicSc2Bot::OnGameStart() {
    int MAX_SCOUTS_PER_QUAD = 2;
    const ObservationInterface *observation = Observation();
    const Unit *start_base =
        observation
            ->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))
            .front();
    initial_hatchery_tag = start_base->tag;
    scout_locations = observation->GetGameInfo().enemy_start_locations;

    // Build drones if not at the given army cap for next item
    build_order.push(BuildOrderItem(
        13, UNIT_TYPEID::ZERG_OVERLORD)); // At 13 cap, build an Overlord
    build_order.push(BuildOrderItem(
        16, UNIT_TYPEID::ZERG_HATCHERY)); // At 16 cap, build a Hatchery
    build_order.push(BuildOrderItem(
        18, UNIT_TYPEID::ZERG_EXTRACTOR)); // At 18 cap, build an Extractor
    build_order.push(BuildOrderItem(
        17,
        UNIT_TYPEID::ZERG_SPAWNINGPOOL)); // At 17 cap, build a Spawning Pool
    build_order.push(BuildOrderItem(
        20, UNIT_TYPEID::ZERG_OVERLORD)); // At 20 cap, build an Overlord
    build_order.push(
        BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN)); // Build 2 Queens
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN));
    build_order.push(
        BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING)); // Build 4 Zerglings
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
    build_order.push(BuildOrderItem(
        32, UNIT_TYPEID::ZERG_OVERLORD)); // At 32 cap, build an Overlord
    build_order.push(
        BuildOrderItem(0, ABILITY_ID::MORPH_LAIR)); // Upgrade Hatchery to Lair
    build_order.push(BuildOrderItem(
        36, UNIT_TYPEID::ZERG_OVERLORD)); // At 36 cap, build an Overlord
    build_order.push(BuildOrderItem(
        0, UNIT_TYPEID::ZERG_ROACHWARREN)); // Build a Roach Warren
    build_order.push(BuildOrderItem(
        0, UNIT_TYPEID::ZERG_EXTRACTOR)); // Build 2 more Extractors
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR));
    build_order.push(BuildOrderItem(
        0, UNIT_TYPEID::ZERG_SPORECRAWLER)); // Build 2 Spore Crawlers
    build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER));

    state.rally_point =
        getValidRallyPoint(observation->GetStartLocation(), Query());

    // find overlord rally location depending on base start location
    Point2D start_location = start_base->pos;
    int map_width = observation->GetGameInfo().width;
    int map_height = observation->GetGameInfo().height;
    std::cout << map_width << " " << map_height;
    int x_half = map_width / 2;
    int y_half = map_height / 2;
    if (start_location.x > x_half) {
        state.overlord_rally_point.x = map_width - 1;
    }
    if (start_location.y > y_half) {
        state.overlord_rally_point.y = map_height - 1;
    }

    Point2D map_center = Point2D(map_width / 2.0f, map_height / 2.0f);
    // add all resource points as scouting locations split up into 4 quadrants
    for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Neutral)) {
        if (unit->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD ||
            unit->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
            Point2D pos = unit->pos;

            if (pos.x >= map_center.x && pos.y > map_center.y) {
                scout_loc_north_east.push_back(pos);
            } else if (pos.x < map_center.x && pos.y > map_center.y) {
                scout_loc_north_west.push_back(pos);
            } else if (pos.x >= map_center.x && pos.y <= map_center.y) {
                scout_loc_south_east.push_back(pos);
            } else if (pos.x < map_center.x && pos.y <= map_center.y) {
                scout_loc_south_west.push_back(pos);
            }
        }
    }
    std::cout << std::endl << "scout nw size: " << scout_loc_north_west.size() << std::endl;
    std::cout << scout_loc_north_east.size() << std::endl;
    std::cout << scout_loc_south_west.size() << std::endl;
    std::cout << scout_loc_south_east.size() << std::endl;
}

void BasicSc2Bot::OnStep() {
    const ObservationInterface *observation = Observation();
    int current_supply = observation->GetFoodUsed();
    int supply_cap = observation->GetFoodCap();
    // Change it to build at cap for example build drones till 13

    // build next structure/unit
    if (!build_order.isEmpty()) {
        auto buildItem = build_order.peek();
        int count;
        if (current_supply >= buildItem.supply) {
            if (tryBuild(buildItem)) {
                build_order.pop();
                std::cout << "Building: " << UnitTypeToName(buildItem.unit_type)
                          << std::endl;
            }
        } else {
            const Unit *larva = nullptr;
            larva = findIdleLarva();
            if (larva && observation->GetMinerals() >= 50 &&
                current_supply != observation->GetFoodCap()) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
            }
        }
    } else { // If build_order queue is empty, we want to build additional
             // drones then start training army
        if (built_drones < additional_drones) {
            const Unit *larva = nullptr;
            larva = findIdleLarva();
            if (larva && observation->GetMinerals() >= 50 &&
                current_supply != observation->GetFoodCap()) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
                std::cout << "Building additional drone: " << built_drones
                          << std::endl;
                ++built_drones;
            }
        }
        // Build roaches until army cap is x-1/x
        // Build an overlord at x-1/x army cap to gain more army space.
        // We ensure not to queue multiple overlords by checking if one is
        // already being built
        if (current_supply >= supply_cap - 1) {
            // First check if overlord is already being built by one of our
            // units
            bool overlord_in_production = false;
            Units my_units = observation->GetUnits(Unit::Alliance::Self);
            for (const auto &unit : my_units) {
                for (const auto &order : unit->orders) {
                    if (order.ability_id == ABILITY_ID::TRAIN_OVERLORD) {
                        overlord_in_production = true;
                        break;
                    }
                }
                if (overlord_in_production) {
                    break;
                }
            }

            // add Overlord to queue if not being built and current_supply is
            // near supply_cap
            if (!overlord_in_production) {
                build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_OVERLORD));
                std::cout
                    << "Queued Overlord in build_order to increase supply."
                    << std::endl;
            } else {
                BuildOrderItem roach_item(0, UNIT_TYPEID::ZERG_ROACH);
                if (tryBuild(roach_item)) {
                }
            }
        } else {
            // When supply is not near cap, keep building Roaches
            BuildOrderItem roach_item(0, UNIT_TYPEID::ZERG_ROACH);
            if (tryBuild(roach_item)) {
            }
        }
    }

    // Scout for enemy bases and add them to out state.enemyBaseLocations
    Units enemy_units = observation->GetUnits(Unit::Alliance::Enemy);
    for (const auto &enemy_unit : enemy_units) {
        // Get UnitTypeData for the enemy unit
        const UnitTypeData &unit_type_data =
            observation->GetUnitTypeData().at(enemy_unit->unit_type);

        // Check if the unit is a structure
        bool is_structure = false;
        for (const auto &attribute : unit_type_data.attributes) {
            if (attribute == Attribute::Structure) {
                is_structure = true;
                break;
            }
        }

        if (is_structure) {
            // Check if this building is already in our list
            bool building_exists = false;
            for (auto &building : state.enemyBaseLocations) {
                if (building.tag == enemy_unit->tag ||
                    positionsAreClose(building.position, enemy_unit->pos,
                                      0.5f)) {
                    building_exists = true;
                    building.tag =
                        enemy_unit->tag; // Updating tag in case it's changing
                    break;
                }
            }
            if (!building_exists) {
                std::cout << "Enemy building located at position: ("
                          << enemy_unit->pos.x << ", " << enemy_unit->pos.y
                          << ")" << std::endl;
                // Add the enemy building to our list
                state.enemyBaseLocations.push_back(
                    {enemy_unit->tag, enemy_unit->pos});
                Units overlords = observation->GetUnits(
                    Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
                for (size_t j = 0; j < overlords.size(); j++) {
                    Actions()->UnitCommand(overlords[j], ABILITY_ID::MOVE_MOVE,
                                           state.overlord_rally_point);
                }
            }
        }
    }
    // for (size_t i = 0; i < enemy_units.size(); i++) {
    //   if ((enemy_units[i]->unit_type == UNIT_TYPEID::ZERG_HATCHERY ||
    //        enemy_units[i]->unit_type == UNIT_TYPEID::PROTOSS_NEXUS ||
    //        enemy_units[i]->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) &&
    //       std::find(state.enemyBaseLocations.begin(),
    //                 state.enemyBaseLocations.end(),
    //                 enemy_units[i]->pos) == state.enemyBaseLocations.end()) {
    //     std::cout << "Base Located!!!";
    //     state.enemyBaseLocations.push_back(enemy_units[i]->pos);
    //     Units overlords = observation->GetUnits(
    //         Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
    //     for (size_t j = 0; j < overlords.size(); j++) {
    //       Actions()->UnitCommand(overlords[j], ABILITY_ID::MOVE_MOVE,
    //                              state.overlord_rally_point);
    //     }
    //   }
    // }

    // build overseers when we are nearing troop capacity
    // buildOverseers();

    // check if we have enough resources to expand to a new base
    //  this would be in the manager
    // if (state.enemyBaseLocations.size() > 0) {
    //   std::cout << "Enemy base location: (" <<
    //   state.enemyBaseLocations.at(0).x << ", " <<
    //   state.enemyBaseLocations.at(0).y << std::endl;
    // }
    // Attack once we have an optimal army build
    // use the attack base queue
    // if (isArmyReady()) {
    //   if (state.enemyBaseLocations.size() > 0) {
    //     std::cout << "Base found: Attacking enemy!!" << std::endl;
    //     launchAttack(state.enemyBaseLocations.at(0));
    //   }
    // }

    // if our current group has >= the desired group_size and we have an enemy
    // base location to attack. Send all roaches in the group to attack
    if (current_roach_group.size() >= group_size &&
        !state.enemyBaseLocations.empty()) {
        Units attack_group;
        for (auto tag : current_roach_group) {
            const Unit *roach = observation->GetUnit(tag);
            if (roach) {
                attack_group.push_back(roach);
            }
        }
        launchAttack(attack_group, state.enemyBaseLocations.at(0).position);
        std::cout << "Sending a group of " << attack_group.size()
                  << " Roaches to attack: ("
                  << state.enemyBaseLocations.at(0).position.x << ", "
                  << state.enemyBaseLocations.at(0).position.y << ")"
                  << std::endl;
        current_roach_group.clear(); // Reset the group for the next wave
    }
    patrolScouts();

    // Queen inject on step since they don't revert back to idle after injecting
    // initially.
    HandleQueenInjects();
    // Balance drones among hatcheries
    BalanceWorkers();

    return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
    state.idleUnits.push_back(unit);
    switch (unit->unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_DRONE: {
        gas_harvesting_drones.erase(unit->tag);
        Units extractors = Observation()->GetUnits(
            Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR));
        const Unit *extractor_to_mine = nullptr;
        for (auto extractor : extractors) {
            int assigned_workers = extractor->assigned_harvesters;
            int max_workers = extractor->ideal_harvesters;

            if (assigned_workers < max_workers && max_workers > 0) {
                extractor_to_mine = extractor;
                break; // Found an Extractor with less than three workers
            }
        }
        if (extractor_to_mine) {
            Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER,
                                   extractor_to_mine);
        } else {
            const Unit *mineral_target =
                FindNearestMineralPatchForHarvest(unit->pos);
            if (!mineral_target) {
                break;
            }
            Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
        }
        break;
    }
    case UNIT_TYPEID::ZERG_OVERLORD: {
        std::cout << "  " << unit->pos.x << "," << unit->pos.y << ", "
                  << inRallyRange(unit->pos, state.overlord_rally_point, 25.0);
        if (scout_locations.size() > 0) {
            Actions()->UnitCommand(unit, ABILITY_ID::SMART,
                                   scout_locations.front());
            scout_locations.erase(scout_locations.begin());
        } else if (!inRallyRange(unit->pos, state.overlord_rally_point, 25.0)) {
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                                   state.overlord_rally_point);
        }
        break;
    }
    case UNIT_TYPEID::ZERG_QUEEN: {
        Units hatcheries =
            Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
        if (!hatcheries.empty()) {
            // Find the closest Hatchery to the Queen
            const Unit *closest_hatchery = nullptr;
            float min_distance = std::numeric_limits<float>::max();
            for (const auto &hatchery : hatcheries) {
                if (hatchery->build_progress == 1.0f) {
                    float distance =
                        DistanceSquared2D(unit->pos, hatchery->pos);
                    if (distance < min_distance) {
                        min_distance = distance;
                        closest_hatchery = hatchery;
                    }
                }
            }
            if (closest_hatchery) {
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA,
                                       closest_hatchery);
            }
        }
        break;
    }
    case UNIT_TYPEID::ZERG_ROACH: {
        // If roach has been sent to attack
        if (attacking_roaches.find(unit->tag) != attacking_roaches.end()) {
            // If there's a base to attack
            if (!state.enemyBaseLocations.empty()) {
                const Point2D &target = state.enemyBaseLocations.at(0).position;
                Actions()->UnitCommand(unit, ABILITY_ID::ATTACK,
                                       target); // Attacks next target instead
                                                // of going to rally point
            } else {
                Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                                       state.rally_point);
            }
        } else {
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                                   state.rally_point);
        }
        break;
    }
    default: {
        break;
    }
    }
}

void BasicSc2Bot::OnBuildingConstructionComplete(const Unit *unit) {
    // If the unit that finished constructing is an Extractor, assign 3 drones
    // to the extractor.
    if (unit->unit_type == UNIT_TYPEID::ZERG_EXTRACTOR) {
        AssignDronesToExtractor(unit);
    }
}

void BasicSc2Bot::OnUnitDestroyed(const Unit *unit) {
    if (unit->unit_type == UNIT_TYPEID::ZERG_DRONE && unit->alliance == sc2::Unit::Alliance::Self) {
        gas_harvesting_drones.erase(unit->tag);
    } else if (unit->unit_type == UNIT_TYPEID::ZERG_OVERLORD && unit->alliance == sc2::Unit::Alliance::Self) {
        build_order.push_front(BuildOrderItem(0, UNIT_TYPEID::ZERG_OVERLORD));
    } else if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH && unit->alliance == sc2::Unit::Alliance::Self) {
        attacking_roaches.erase(unit->tag); // Clean up attacking_roaches
    } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING && unit->alliance == sc2::Unit::Alliance::Self) {
        std::cout << "scout killed" << std::endl;
        scouts_nw.erase(unit->tag);
        scouts_ne.erase(unit->tag);
        scouts_sw.erase(unit->tag);
        scouts_se.erase(unit->tag);
        scouts.erase(unit->tag);
    }

    // Check if an enemy building has been destroyed
    if (unit->alliance == Unit::Alliance::Enemy) {
        const ObservationInterface *observation = Observation();
        const UnitTypeData &unit_type_data =
            observation->GetUnitTypeData().at(unit->unit_type);

        // Check if the unit was a structure
        bool is_structure = false;
        for (const auto &attribute : unit_type_data.attributes) {
            if (attribute == Attribute::Structure) {
                is_structure = true;
                break;
            }
        }

        if (is_structure) {
            // Remove the building from enemyBaseLocations
            auto it = std::remove_if(
                state.enemyBaseLocations.begin(),
                state.enemyBaseLocations.end(),
                [unit, this](const GameManager::EnemyBuilding &building) {
                    return positionsAreClose(building.position, unit->pos,
                                             0.5f);
                });
            if (it != state.enemyBaseLocations.end()) {
                state.enemyBaseLocations.erase(it,
                                               state.enemyBaseLocations.end());
                std::cout << "Enemy building destroyed! Removed from building "
                             "locations."
                          << std::endl;
            }
        }
    }
}

void BasicSc2Bot::OnUnitCreated(const Unit *unit) {
    switch (unit->unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_ROACH: {
        // Add Roach to the current group
        current_roach_group.push_back(unit->tag);
        std::cout << "Roach created and added to current group. Group size: "
                  << current_roach_group.size() << std::endl;
        // Move Roach to the rally point
        Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, state.rally_point);
        break;
    }
    case sc2::UNIT_TYPEID::ZERG_ZERGLING: {
        std::vector<Point2D> *scout_points;
        std::set<Tag> *scouts_at_quad;
        bool added_scout = false;
        if (scouts_nw.size() < 2) {
            scout_points = &scout_loc_north_west;
            scouts_at_quad = &scouts_nw;
            added_scout = true;
        } else if (scouts_ne.size() < 2) {
            scout_points = &scout_loc_north_east;
            scouts_at_quad = &scouts_ne;
            added_scout = true;
        } else if (scouts_sw.size() < 2) {
            scout_points = &scout_loc_south_west;
            scouts_at_quad = &scouts_sw;
            added_scout = true;
        } else if (scouts_se.size() < 2) {
            scout_points = &scout_loc_south_east;
            scouts_at_quad = &scouts_se;
            added_scout = true;
        }
        if (added_scout) {
            // get scout info
            // add index of scout
            // add it to scout set
            int index = GetRandomInteger(0, scout_points->size() - 1);
            scouts.insert(std::pair<Tag, Scout>(
                unit->tag, Scout(unit->tag, index)));
            scouts_at_quad->insert(unit->tag);
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                                   scout_points->at(index));
                                  std::cout << "added scout: " << scouts.size() << "moving to idx: " << index << " x:" << scout_points->at(index).x << std::endl;
        }
        break;
    }
    default:{

    }
    }
}

void BasicSc2Bot::OnGameEnd() {
    const auto &results = Observation()->GetResults();
    for (const auto &result : results) {
        if (result.player_id == Observation()->GetPlayerID()) {
            switch (result.result) {
            case sc2::GameResult::Win:
                std::cout << "Game ended: Victory!" << std::endl;
                break;
            case sc2::GameResult::Loss:
                std::cout << "Game ended: Defeat." << std::endl;
                break;
            case sc2::GameResult::Tie:
                std::cout << "Game ended: Tie." << std::endl;
                break;
            default:
                std::cout << "Game ended: Unknown result." << std::endl;
                break;
            }
        }
    }
}

const Point2D BasicSc2Bot::getValidRallyPoint(const Point2D &base_position,
                                              QueryInterface *query,
                                              float max_radius, float step) {
    // Spiral pattern starting from the base position
    for (float radius = step; radius <= max_radius; radius += step) {
        for (float angle = 0.0f; angle < 360.0f;
             angle += 45.0f) { // Check 8 directions in each radius
            float radians = angle * M_PI / 180.0f;
            Point2D potential_point =
                Point2D(base_position.x + radius * std::cos(radians),
                        base_position.y + radius * std::sin(radians));
            // Check if the point is valid for placement
            if (query->Placement(ABILITY_ID::RALLY_UNITS, potential_point)) {
                return potential_point; // Return the first valid point
            }
        }
    }
    // If no valid point is found, fallback to the base position
    return base_position;
}

const Unit *BasicSc2Bot::FindNearestMineralPatch(const Point2D &start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const Unit *target = nullptr;
    for (const auto &u : units) {
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD &&
            mineral_locations.find(u) == mineral_locations.end()) {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    mineral_locations.insert(target);
    return target;
}

const Unit *
BasicSc2Bot::FindNearestMineralPatchForHarvest(const Point2D &start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const Unit *target = nullptr;
    for (const auto &u : units) {
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD &&
            mineral_locations.find(u) == mineral_locations.end()) {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

struct IsVespeneGeyser {
    bool operator()(const Unit &unit) {
        return unit.unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER ||
               unit.unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER ||
               unit.unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
               unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
               unit.unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER ||
               unit.unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER;
    }
};

const Unit *BasicSc2Bot::FindNearestVespenePatch(const Point2D &start) {
    Units geysers =
        Observation()->GetUnits(Unit::Alliance::Neutral, IsVespeneGeyser());
    float distance = std::numeric_limits<float>::max();
    const Unit *target = nullptr;
    for (const auto &geyser : geysers) {
        // Check if the geyser already has an Extractor built on it
        bool has_extractor = false;
        for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
            if (unit->unit_type == UNIT_TYPEID::ZERG_EXTRACTOR &&
                DistanceSquared2D(unit->pos, geyser->pos) < 1.0f) {
                has_extractor = true;
                break;
            }
        }
        if (!has_extractor) {
            float d = DistanceSquared2D(geyser->pos, start);
            if (d < distance) {
                distance = d;
                target = geyser;
            }
        }
    }
    return target;
}

const Unit *BasicSc2Bot::findAvailableDrone() {
    Units drones = Observation()->GetUnits(Unit::Alliance::Self,
                                           IsUnit(UNIT_TYPEID::ZERG_DRONE));

    for (const auto &drone : drones) {
        // Skip gas harvesting drones
        if (gas_harvesting_drones.find(drone->tag) !=
            gas_harvesting_drones.end()) {
            continue;
        }
        // Prioritize idle drones
        if (drone->orders.empty()) {
            return drone;
        }
        // Secondary, get a drone that is harvesting minerals.
        for (const auto &order : drone->orders) {
            if (order.ability_id == ABILITY_ID::HARVEST_GATHER ||
                order.ability_id == ABILITY_ID::HARVEST_RETURN) {
                return drone;
            }
        }
    }

    // As a last resort, find any Drone not building something
    for (const auto &drone : drones) {
        if (!drone->orders.empty()) {
            AbilityID current_ability = drone->orders.front().ability_id;
            if (current_ability != ABILITY_ID::BUILD_HATCHERY &&
                current_ability != ABILITY_ID::BUILD_EXTRACTOR &&
                current_ability != ABILITY_ID::BUILD_SPAWNINGPOOL) {
                return drone;
            }
        }
    }

    // No available Drones found
    return nullptr;
}
const Unit *BasicSc2Bot::findAvailableLarva() {
    Units larvas = Observation()->GetUnits(Unit::Alliance::Self,
                                           IsUnit(UNIT_TYPEID::ZERG_LARVA));

    // Prioritize idle Drones
    for (const auto &larva : larvas) {
        if (larva->orders.empty()) {
            return larva;
        }
    }

    // If no idle Drones, find one that's harvesting minerals
    for (const auto &larva : larvas) {
        if (!larva->orders.empty()) {
            AbilityID current_ability = larva->orders.front().ability_id;
            if (current_ability == ABILITY_ID::HARVEST_GATHER ||
                current_ability == ABILITY_ID::HARVEST_RETURN) {
                return larva;
            }
        }
    }

    // As a last resort, find any Drone not building something
    for (const auto &larva : larvas) {
        if (!larva->orders.empty()) {
            AbilityID current_ability = larva->orders.front().ability_id;
            if (current_ability != ABILITY_ID::BUILD_HATCHERY &&
                current_ability != ABILITY_ID::BUILD_EXTRACTOR &&
                current_ability != ABILITY_ID::BUILD_SPAWNINGPOOL) {
                return larva;
            }
        }
    }

    // No available larvas found
    return nullptr;
}
Point2D
BasicSc2Bot::findBuildPositionNearMineral(const Point2D &target_position) {
    const float hatchery_size = 5.0f; // Hatchery size in grid squares
    const float build_radius = hatchery_size / 2.0f; // Half the size for radius
    const float search_radius =
        build_radius + 4.0f;        // Add extra space for clearance
    const float angle_step = 10.0f; // Angle increment in degrees

    const ObservationInterface *observation = Observation();

    for (float radius = search_radius; radius <= search_radius + 5.0f;
         radius += 1.0f) {
        for (float angle = 0.0f; angle < 360.0f; angle += angle_step) {
            float radians = angle * 3.14159f / 180.0f;
            Point2D build_position =
                Point2D(target_position.x + radius * cos(radians),
                        target_position.y + radius * sin(radians));

            // Check if the position is valid for building a Hatchery
            if (Query()->Placement(ABILITY_ID::BUILD_HATCHERY,
                                   build_position)) {
                return build_position;
            }
        }
    }

    // No valid position found
    return Point2D(0.0f, 0.0f);
}
Point2D BasicSc2Bot::findBuildPosition(const Point2D &start) {
    float search_radius = 5.0f;
    float search_increment = 2.0f;

    // Iterate through a spiral or grid pattern around the base position
    for (float x = -search_radius; x <= search_radius; x += search_increment) {
        for (float y = -search_radius; y <= search_radius;
             y += search_increment) {
            Point2D test_location = start + Point2D(x, y);

            // Check if the position is buildable for the Spawning Pool
            if (Query()->Placement(ABILITY_ID::BUILD_SPAWNINGPOOL,
                                   test_location)) {
                return test_location;
            }
        }
    }
}
const Unit *BasicSc2Bot::findIdleLarva() {
    for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_LARVA &&
            unit->orders.empty()) {
            return unit;
        }
    }
    return nullptr;
}
const Unit *BasicSc2Bot::findIdleDrone() {
    for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_DRONE &&
            unit->orders.empty()) {
            return unit;
        }
    }
    return nullptr;
}

// Eventually will use manager
// For now it checks whether its a unit, strucure, or upgrade
bool BasicSc2Bot::tryBuild(struct BuildOrderItem buildItem) {
    const ObservationInterface *observation = Observation();

    // Check if the item is a MORPH_LAIR upgrade to handle separately.
    if (buildItem.ability == ABILITY_ID::MORPH_LAIR) {
        const Unit *initial_hatchery =
            observation->GetUnit(initial_hatchery_tag);
        if (initial_hatchery && initial_hatchery->build_progress == 1.0f &&
            initial_hatchery->orders.empty()) {
            if (observation->GetMinerals() >= 150 &&
                observation->GetVespene() >= 100) {
                Actions()->UnitCommand(initial_hatchery,
                                       ABILITY_ID::MORPH_LAIR);
                std::cout << "Upgrading initial hatchery to Lair." << std::endl;
                return true;
            } else {
                std::cout << "Insufficient resources to morph Lair."
                          << std::endl;
            }
        } else {
            std::cout << "Initial hatchery not ready for Lair upgrade."
                      << std::endl;
        }
        return false;
    }

    // if its a unit build it if we can afford
    if (buildItem.is_unit) {
        // check whether its a unit or a structure
        const Unit *larva = nullptr;
        switch (buildItem.unit_type) {
        case UNIT_TYPEID::ZERG_DRONE:
            larva = findIdleLarva();
            if (larva) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
                return true;
            }
            break;

        case UNIT_TYPEID::ZERG_OVERLORD:
            larva = findIdleLarva();
            if (larva && observation->GetMinerals() >= 100) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_OVERLORD);
                return true;
            }
            break;

        case UNIT_TYPEID::ZERG_ROACH: {
            // Check if roach warren is built
            const Units roach_warrens = observation->GetUnits(
                Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_ROACHWARREN));
            bool roach_warren_done = false;
            for (const auto &roach_warren : roach_warrens) {
                if (roach_warren->build_progress == 1.0f) {
                    roach_warren_done = true;
                    break;
                }
            }
            if (!roach_warren_done) {
                // Cannot build roach without roach warren
                return false;
            }

            larva = findIdleLarva();
            int current_supply = observation->GetFoodUsed();
            int supply_cap = observation->GetFoodCap();
            if (larva && observation->GetMinerals() >= 75 &&
                observation->GetVespene() >= 25 &&
                current_supply < supply_cap) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ROACH);
                return true;
            }
            break;
        }
        case UNIT_TYPEID::ZERG_ZERGLING: {
            const Units spawning_pools = observation->GetUnits(
                Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL));
            bool spawning_pool_done = false;
            for (const auto &spawning_pool : spawning_pools) {
                if (spawning_pool->build_progress == 1.0f) {
                    // Spawning Pool is fully constructed
                    spawning_pool_done = true;
                }
            }
            if (!spawning_pool_done) {
                return false;
            }
            larva = findAvailableLarva();
            if (larva && observation->GetMinerals() >= 50) {
                Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ZERGLING);
                return true;
            }
            break;
        }

        case UNIT_TYPEID::ZERG_QUEEN: {
            const Units spawning_pools = observation->GetUnits(
                Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL));
            bool spawning_pool_done = false;
            for (const auto &spawning_pool : spawning_pools) {
                if (spawning_pool->build_progress == 1.0f) {
                    // Spawning Pool is fully constructed
                    spawning_pool_done = true;
                }
            }
            if (!spawning_pool_done) {
                return false;
            }
            for (const auto &hatchery :
                 observation->GetUnits(Unit::Alliance::Self,
                                       IsUnit(UNIT_TYPEID::ZERG_HATCHERY))) {
                if (hatchery->build_progress == 1.0f &&
                    hatchery->orders.empty() &&
                    observation->GetMinerals() >= 150) {
                    Actions()->UnitCommand(hatchery, ABILITY_ID::TRAIN_QUEEN);
                    return true;
                }
            }
            break;
        }

        // if its a hatchery we get the nearest mineral location that we havent
        // visited
        case UNIT_TYPEID::ZERG_HATCHERY: {
            const Unit *drone = findAvailableDrone();
            if (drone && observation->GetMinerals() >= 300) {
                std::cout << "building hatchery..." << std::endl;
                // locate the next nearest mineral group
                const Unit *mineral_cluster_a =
                    FindNearestMineralPatch(drone->pos);
                const Unit *mineral_cluster_b =
                    findNextNearestMineralGroup(mineral_cluster_a);
                std::cout << "located mineral cluster: "
                          << mineral_cluster_b->pos.x << " , "
                          << mineral_cluster_b->pos.y << std::endl;
                // get map center
                Point2D map_center = getMapCenter();
                // get vector from cluster center to map center, normalize into
                // a direction vector
                Point2D direction_vector(
                    getDirectionVector(mineral_cluster_b->pos, map_center));
                // create point for hatchery
                std::cout << "direction vector: " << direction_vector.x << " , "
                          << direction_vector.y << std::endl;
                Point2D hatchery_location(
                    mineral_cluster_b->pos.x + (direction_vector.x * 3.0f),
                    mineral_cluster_b->pos.y + (direction_vector.y * 3.0f));
                std::cout << "calculated hatchery point: "
                          << hatchery_location.x << " , " << hatchery_location.y
                          << std::endl;
                // find build position
                Point2D build_position =
                    findBuildPositionNearMineral(hatchery_location);
                std::cout << "build position: " << build_position.x << " , "
                          << build_position.y << std::endl;
                if (build_position.x != 0.0f || build_position.y != 0.0f) {
                    Actions()->UnitCommand(drone, ABILITY_ID::BUILD_HATCHERY,
                                           build_position);
                    std::cout << "hatchery built." << std::endl;
                    return true;
                }
            }
            break;
        }

        case UNIT_TYPEID::ZERG_EXTRACTOR: {
            const Unit *drone = findAvailableDrone();
            if (drone && observation->GetMinerals() >= 25) {
                const Unit *target_hatchery = nullptr;
                // If we're building the 3rd extractor
                if (built_extractors == 2) {
                    Units hatcheries = observation->GetUnits(
                        Unit::Alliance::Self, IsTownHall());
                    for (const auto &hatchery : hatcheries) {
                        if (hatchery->build_progress == 1.0f &&
                            hatchery->tag != initial_hatchery_tag) {
                            target_hatchery = hatchery;
                            break;
                        }
                    }
                } else {
                    // If building first 2 extractors, we can use the position
                    // of the initial hatchery
                    target_hatchery =
                        observation->GetUnit(initial_hatchery_tag);
                }
                if (target_hatchery) {
                    const Unit *nearest_vespene =
                        FindNearestVespenePatch(target_hatchery->pos);
                    if (nearest_vespene) {
                        Actions()->UnitCommand(drone,
                                               ABILITY_ID::BUILD_EXTRACTOR,
                                               nearest_vespene);
                        ++built_extractors;
                        return true;
                    }
                }
                // const Unit *nearest_vespene_loc =
                // FindNearestVespenePatch(drone->pos);

                // Actions()->UnitCommand(drone, ABILITY_ID::BUILD_EXTRACTOR,
                //                        nearest_vespene_loc);
                // return true;
            }
            break;
        }
        case UNIT_TYPEID::ZERG_SPAWNINGPOOL: {
            const Unit *drone = findAvailableDrone();
            if (drone && observation->GetMinerals() >= 200) {
                float rx = GetRandomScalar();
                float ry = GetRandomScalar();
                Point2D build_position = findBuildPosition(drone->pos);
                Actions()->UnitCommand(drone, ABILITY_ID::BUILD_SPAWNINGPOOL,
                                       build_position);
                return true;
            }
            break;
        }
        case UNIT_TYPEID::ZERG_ROACHWARREN: {
            const Unit *drone = findAvailableDrone();
            if (drone && observation->GetMinerals() >= 150) {
                AbilityID build_ability = ABILITY_ID::BUILD_ROACHWARREN;
                Point2D build_position =
                    FindPlacementLocation(build_ability, drone->pos);
                if (build_position != Point2D(0.0f, 0.0f)) {
                    Actions()->UnitCommand(drone, build_ability,
                                           build_position);
                    std::cout << "Building Roach Warren" << std::endl;
                    return true;
                } else {
                    std::cout << "Can't find location for Roach Warren"
                              << std::endl;
                }
            }
            break;
        }
        case UNIT_TYPEID::ZERG_SPORECRAWLER: {
            const Unit *drone = findAvailableDrone();
            if (drone && observation->GetMinerals() >= 75) {
                AbilityID build_ability = ABILITY_ID::BUILD_SPORECRAWLER;
                Point2D build_position =
                    FindPlacementLocation(build_ability, drone->pos);
                if (build_position != Point2D(0.0f, 0.0f)) {
                    Actions()->UnitCommand(drone, build_ability,
                                           build_position);
                    return true;
                }
            }
            break;
        }

        default:
            const Unit *drone = findAvailableDrone();
            if (drone) {
                float rx = GetRandomScalar();
                float ry = GetRandomScalar();
                Point2D build_position =
                    Point2D(drone->pos.x - 50, drone->pos.y - 100);

                Actions()->UnitCommand(drone, buildItem.ability,
                                       build_position);
                return true;
            }
        }
        return false;
    }
    // must be an upgrade
    if (buildItem.ability != ABILITY_ID::INVALID && !buildItem.is_unit) {
        Units structures = observation->GetUnits(Unit::Alliance::Self,
                                                 IsUnit(buildItem.unit_type));
        if (!structures.empty()) {
            Actions()->UnitCommand(structures[0], buildItem.ability);
            return true;
        }
    }
    return false;
}

bool BasicSc2Bot::isArmyReady() {
    int roach_count = 0;
    int zergling_count = 0;
    const int optRoach = 3;    // -r x
    const int optZergling = 0; // -z x
    for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
            roach_count++;
        } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING) {
            zergling_count++;
        }
    }
    return roach_count >= optRoach && zergling_count >= optZergling;
}

// void BasicSc2Bot::launchAttack(const Point2D &target) {
//   for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
//     if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING ||
//         unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
//       Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
//     }
//   }
// }
void BasicSc2Bot::launchAttack(const Units &attack_group,
                               const Point2D &target) {
    Actions()->UnitCommand(attack_group, ABILITY_ID::ATTACK,
                           target); // Send group of roaches to attack
    // Add roach tags to attacking_roaches set
    for (const auto &unit : attack_group) {
        attacking_roaches.insert(unit->tag);
    }
}

// Function to calculate the center of the map
Point2D BasicSc2Bot::getMapCenter() const {
    // Get the playable map area
    const sc2::GameInfo &game_info = Observation()->GetGameInfo();
    sc2::Point2D playable_min = game_info.playable_min;
    sc2::Point2D playable_max = game_info.playable_max;

    // Calculate the center point
    float center_x = (playable_min.x + playable_max.x) / 2.0f;
    float center_y = (playable_min.y + playable_max.y) / 2.0f;

    return sc2::Point2D(center_x, center_y);
}
void BasicSc2Bot::patrolScouts() {
    for (auto &scout_pair : scouts) {
        Scout &scout = scout_pair.second;
        const Unit *unit = Observation()->GetUnit(scout.unit_tag);
        const std::vector<Point2D>* points;
        if (scouts_nw.find(scout.unit_tag) != scouts_nw.end()) {
            points = &scout_loc_north_west;
        }
        else if (scouts_ne.find(scout.unit_tag) != scouts_ne.end()) {
            points = &scout_loc_north_east;
        }
        else if (scouts_sw.find(scout.unit_tag) != scouts_sw.end()) {
            points = &scout_loc_south_west;
        }
        else if (scouts_se.find(scout.unit_tag) != scouts_se.end()) {
            points = &scout_loc_south_east;
        }
        // Check if the scout is close to its current target
        Point2D target = points->at(scout.current_target_idx);
        if (DistanceSquared2D(unit->pos, target) <
            4.0f) { // Threshold for "reached"
            // Move to the next resource point in the list
            scout.current_target_idx =
                (scout.current_target_idx + 1) % points->size();
            target = points->at(scout.current_target_idx);
            // Command the scout to move to the target
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, target);
        }
    }
}

const Unit *
BasicSc2Bot::findNextNearestMineralGroup(const Unit *mineral_loc_a) {
    /**
     * finds and returns a mineral patch in the next nearest group of mineral
     * patches
     */
    const Unit *mineral_loc_b = FindNearestMineralPatch(mineral_loc_a->pos);
    std::cout << "differences: ";
    for (int i = 0; i < 10; i++) {
        // compare differences
        if (getVectorDifferenceMagnitude(mineral_loc_a->pos,
                                         mineral_loc_b->pos) > 20.00) {
            // mineral_loc_b is further from previous minerals by a factor of 10
            return mineral_loc_b;
        }
        // get next mineral pairs
        mineral_loc_a = mineral_loc_b;
        mineral_loc_b = FindNearestMineralPatch(mineral_loc_b->pos);
    }
    std::cout << std::endl;
    return mineral_loc_b; // in case of next group not found within 10 minerals,
                          // return 10th mineral patch
}
double BasicSc2Bot::getVectorDifferenceMagnitude(Point2D vec_a, Point2D vec_b) {
    /**
     * calculates and returns the magnitude of the difference between two points
     */
    Point2D difference_vector(vec_a.x - vec_b.x, vec_a.y - vec_b.y);
    double diff = sqrt(difference_vector.x * difference_vector.x +
                       difference_vector.y * difference_vector.y);
    std::cout << diff << ", ";
    return diff;
}
Point2D BasicSc2Bot::getDirectionVector(Point2D vec_a, Point2D vec_b) {
    /**
     * calculates and returns the direction vector of the line from point a to
     * point b
     */
    Point2D direction_vector(vec_b.x - vec_a.x, vec_b.y - vec_a.y);
    double direction_vector_magnitude =
        1.00f / sqrt(direction_vector.x * direction_vector.x +
                     direction_vector.y * direction_vector.y);
    Point2D normalized_direction_vector(
        direction_vector_magnitude * direction_vector.x,
        direction_vector_magnitude * direction_vector.y);
    return normalized_direction_vector;
}

bool BasicSc2Bot::inRallyRange(const Point2D &pos, const Point2D &rally,
                               float range) {
    std::cout << "     pos.x: " << pos.x << "pos.y: " << pos.y
              << "rally.x: " << rally.x << "rally.y: " << rally.y << "     ";
    if (abs(pos.x - rally.x) < range && abs(pos.y - rally.y) < range) {
        return true;
    } else {
        return false;
    }
}

Point2D BasicSc2Bot::FindPlacementLocation(AbilityID ability,
                                           const Point2D &near_point) {
    // Try random points around the near_point
    for (int i = 0; i < 20; ++i) { // Increased iterations for better chances
        float rx = GetRandomScalar() * 10.0f - 5.0f; // Center around near_point
        float ry = GetRandomScalar() * 10.0f - 5.0f;
        Point2D test_point = near_point + Point2D(rx, ry);

        // Check if the position is valid for building
        if (Query()->Placement(ability, test_point) &&
            Observation()->HasCreep(test_point)) {
            return test_point;
        }
    }
    // No valid position found
    return Point2D(0.0f, 0.0f); // Indicates failure
}

// To be called onStep() to make sure queens are injecting as soon as they can
void BasicSc2Bot::HandleQueenInjects() {
    const ObservationInterface *observation = Observation();
    Units queens = observation->GetUnits(Unit::Alliance::Self,
                                         IsUnit(UNIT_TYPEID::ZERG_QUEEN));
    Units hatcheries =
        observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    // Loop through all the queens to find one that is available to inject.
    for (const auto &queen : queens) {
        if (queen->energy >= 25) {
            // Check if it's already mid injection to not interrupt.
            bool injecting = false;
            for (const auto &order : queen->orders) {
                if (order.ability_id == ABILITY_ID::EFFECT_INJECTLARVA) {
                    injecting = true;
                    break;
                }
            }
            // If injecting, go to next queen
            if (injecting) {
                continue;
            }

            if (!hatcheries.empty()) {
                // Find the closest Hatchery to the Queen
                const Unit *closest_hatchery = nullptr;
                float min_distance = std::numeric_limits<float>::max();
                for (const auto &hatchery : hatcheries) {
                    if (hatchery->build_progress == 1.0f) {
                        float distance =
                            DistanceSquared2D(queen->pos, hatchery->pos);
                        if (distance < min_distance) {
                            min_distance = distance;
                            closest_hatchery = hatchery;
                        }
                    }
                }
                // Inject closest hatchery
                if (closest_hatchery) {
                    Actions()->UnitCommand(queen,
                                           ABILITY_ID::EFFECT_INJECTLARVA,
                                           closest_hatchery);
                    std::cout << "Queen Injecting!" << std::endl;
                }
            }
        }
    }
}

// Used in OnBuildingConstructionComplete to assign 3 drones to an extractor as
// soon as it's built
void BasicSc2Bot::AssignDronesToExtractor(const Unit *extractor) {
    const ObservationInterface *observation = Observation();
    std::set<Tag>
        assigned_drones; // Set to track which drones have already been assigned
                         // to not duplicate call the same drone.
    int drone_count = 0;
    while (drone_count < 3) {
        const Unit *drone = findAvailableDrone();
        if (!drone) {
            std::cout << "No more available drones to assign." << std::endl;
            break;
        }
        // Checks if drone is a valid unit and whether the drone's tag is
        // already in the set of assigned drones.
        if (drone &&
            (assigned_drones.find(drone->tag) == assigned_drones.end())) {
            Actions()->UnitCommand(drone, ABILITY_ID::HARVEST_GATHER,
                                   extractor);
            assigned_drones.insert(drone->tag);
            // Adding Tag to global set so findAvailableDrone() doesn't grab gas
            // harvesting drones.
            gas_harvesting_drones.insert(drone->tag);
            std::cout << "Drone assigned to extractor!" << std::endl;
            ++drone_count;
        }
    }
}

bool BasicSc2Bot::positionsAreClose(const Point2D &a, const Point2D &b,
                                    float tolerance = 0.5f) {
    return DistanceSquared2D(a, b) <= tolerance * tolerance;
}

void BasicSc2Bot::BalanceWorkers() {
    const ObservationInterface *observation = Observation();
    Units hatcheries =
        observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    std::vector<const Unit *>
        overSaturatedBases; // Bases with more assigned_workers than
                            // ideal_workers
    std::vector<const Unit *>
        underSaturatedBases; // Bases with more ideal_workers than
                             // assigned_workers

    // Iterate to check if a hatchery is over or under saturated
    for (const auto &base : hatcheries) {
        if (base->build_progress < 1.0f) {
            continue; // Ignore incomplete hatcheries
        }

        int assigned_workers = base->assigned_harvesters;
        int ideal_workers = base->ideal_harvesters;
        if (assigned_workers > ideal_workers) {
            overSaturatedBases.push_back(base);
        } else if (assigned_workers < ideal_workers) {
            underSaturatedBases.push_back(base);
        }
    }
    // Exit loop if there's no over or under saturated bases. If one is empty,
    // there's no reassigning to be done.
    if (overSaturatedBases.empty() || underSaturatedBases.empty()) {
        return;
    }
    // Get correct amount of additional drones from over saturated base to send
    // to under saturated base
    for (const auto &overBase : overSaturatedBases) {
        int additional_workers =
            overBase->assigned_harvesters - overBase->ideal_harvesters;
        Units drones = observation->GetUnits(Unit::Alliance::Self,
                                             IsUnit(UNIT_TYPEID::ZERG_DRONE));
        std::vector<const Unit *> dronesToTransfer;

        // Find the drones that are gathering minerals for this base
        for (const auto &drone : drones) {
            if (drone->orders.empty()) {
                continue; // Ignore if the drone is idle
            }
            const Unit *target =
                observation->GetUnit(drone->orders.front().target_unit_tag);
            if (!target) {
                continue; // If target doesnt exist, ignore
            }

            if (target->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD &&
                (DistanceSquared2D(target->pos, overBase->pos) < 100.0f)) {
                dronesToTransfer.push_back(
                    drone); // If it's gathering minerals and near the hatchery,
                            // transfer it
                if (dronesToTransfer.size() > additional_workers) {
                    break; // Stop gathering drones to transfer if we found
                           // enough.
                }
            }
        }

        // Transfer drones to under saturated base
        for (const auto &drone : dronesToTransfer) {
            const Unit *underBase = underSaturatedBases.at(0);
            if (underBase) {
                const Unit *mineral_patch =
                    FindNearestMineralPatchForHarvest(underBase->pos);
                if (mineral_patch) {
                    Actions()->UnitCommand(drone, ABILITY_ID::HARVEST_GATHER,
                                           mineral_patch);
                    std::cout
                        << "Transferring drone to under saturated hatchery!!"
                        << std::endl;
                }
            }
        }
    }
}