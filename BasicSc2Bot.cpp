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
#include <utility>
#define M_PI 3.14

using namespace sc2;

GameManager state;
int MAX_SCOUTS_PER_QUAD = 1;

// void BasicSc2Bot::OnGameStart() {
//   const ObservationInterface *observation = Observation();
//   // Initialize the unscouted mineral patches vector
//   Units mineral_patches = Observation()->GetUnits(
//       Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));
//   for (const auto &mineral_patch : mineral_patches) {
//     unscouted_mineral_patches.push_back(mineral_patch->pos);
//   }

//   // Initialize info about our starting hatchery.
//   const Unit *start_base =
//       observation
//           ->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))
//           .front();
//   initial_hatchery_tag = start_base->tag;
//   scout_locations = observation->GetGameInfo().enemy_start_locations;

//   // Build drones if not at the given army cap for next item
//   build_order.push(BuildOrderItem(
//       13, UNIT_TYPEID::ZERG_OVERLORD)); // At 13 cap, build an Overlord
//   build_order.push(BuildOrderItem(
//       16, UNIT_TYPEID::ZERG_HATCHERY)); // At 16 cap, build a Hatchery
//   build_order.push(BuildOrderItem(
//       18, UNIT_TYPEID::ZERG_EXTRACTOR)); // At 18 cap, build an Extractor
//   build_order.push(BuildOrderItem(
//       17,
//       UNIT_TYPEID::ZERG_SPAWNINGPOOL)); // At 17 cap, build a Spawning Pool
//   build_order.push(BuildOrderItem(
//       20, UNIT_TYPEID::ZERG_OVERLORD)); // At 20 cap, build an Overlord
//   build_order.push(
//       BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN)); // Build 2 Queens
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_QUEEN));
//   build_order.push(
//       BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING)); // Build 4 Zerglings
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_ZERGLING));
//   build_order.push(BuildOrderItem(
//       32, UNIT_TYPEID::ZERG_OVERLORD)); // At 32 cap, build an Overlord
//   build_order.push(
//       BuildOrderItem(0, ABILITY_ID::MORPH_LAIR)); // Upgrade Hatchery to Lair
//   build_order.push(BuildOrderItem(
//       36, UNIT_TYPEID::ZERG_OVERLORD)); // At 36 cap, build an Overlord
//   build_order.push(
//       BuildOrderItem(0, UNIT_TYPEID::ZERG_ROACHWARREN)); // Build a Roach Warren
//   build_order.push(BuildOrderItem(
//       0, UNIT_TYPEID::ZERG_EXTRACTOR)); // Build 2 more Extractors
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR));
//   build_order.push(BuildOrderItem(
//       0, UNIT_TYPEID::ZERG_SPORECRAWLER)); // Build 2 Spore Crawlers
//   build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER));

//   state.rally_point =
//       getValidRallyPoint(observation->GetStartLocation(), Query());

//   // find overlord rally location depending on base start location
//   Point2D start_location = start_base->pos;
//   int map_width = observation->GetGameInfo().width;
//   int map_height = observation->GetGameInfo().height;
//   int x_half = map_width / 2;
//   int y_half = map_height / 2;
//   if (start_location.x > x_half) {
//     state.overlord_rally_point.x = map_width - 1;
//   }
//   if (start_location.y > y_half) {
//     state.overlord_rally_point.y = map_height - 1;
//   }

//   Point2D map_center = Point2D(map_width / 2.0f, map_height / 2.0f);
//   // add all resource points as scouting locations split up into 4 quadrants
//   for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Neutral)) {
//     if (unit->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD ||
//         unit->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
//       Point2D pos = unit->pos;

//       if (pos.x >= map_center.x && pos.y > map_center.y) {
//         scout_loc_north_east.push_back(pos);
//       } else if (pos.x < map_center.x && pos.y > map_center.y) {
//         scout_loc_north_west.push_back(pos);
//       } else if (pos.x >= map_center.x && pos.y <= map_center.y) {
//         scout_loc_south_east.push_back(pos);
//       } else if (pos.x < map_center.x && pos.y <= map_center.y) {
//         scout_loc_south_west.push_back(pos);
//       }
//     }
//   }
// }

void BasicSc2Bot::InitializeMineralPatches() {
    const ObservationInterface *observation = Observation();
    Units mineral_patches = observation->GetUnits(
        Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));

    for (const auto &mineral_patch : mineral_patches) {
    unscouted_mineral_patches.push_back(mineral_patch->pos);
  }
  initialized_mineral_patches = true;
}

void BasicSc2Bot::InitializeStartingHatchery() {
  const ObservationInterface *observation = Observation();
  // Initialize info about our starting hatchery.
  const Unit *start_base =
      observation
          ->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))
          .front();
  initial_hatchery_tag = start_base->tag;
  scout_locations = observation->GetGameInfo().enemy_start_locations;

  // find overlord rally location depending on base start location
  Point2D start_location = start_base->pos;
  int map_width = observation->GetGameInfo().width;
  int map_height = observation->GetGameInfo().height;
  int x_half = map_width / 2;
  int y_half = map_height / 2;
  if (start_location.x > x_half) {
    state.overlord_rally_point.x = map_width - 1;
  }
  if (start_location.y > y_half) {
    state.overlord_rally_point.y = map_height - 1;
  }

  Point2D map_center = Point2D(map_width / 2.0f, map_height / 2.0f);
  initialized_hatchery = true;
}

void BasicSc2Bot::InitializeBuildOrderAndScouts() {
  const ObservationInterface *observation = Observation();
  scout_locations = observation->GetGameInfo().enemy_start_locations;
  for (int i = 0; i < scout_locations.size(); ++i) {
    std::cout << "Scout location for overlord: " << scout_locations.at(i).x << ", " << scout_locations.at(i).y << std::endl;
  }

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
  build_order.push(
      BuildOrderItem(0, UNIT_TYPEID::ZERG_ROACHWARREN)); // Build a Roach Warren
  build_order.push(BuildOrderItem(
      0, UNIT_TYPEID::ZERG_EXTRACTOR)); // Build 2 more Extractors
  build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_EXTRACTOR));
  build_order.push(BuildOrderItem(
      0, UNIT_TYPEID::ZERG_SPORECRAWLER)); // Build 2 Spore Crawlers
  build_order.push(BuildOrderItem(0, UNIT_TYPEID::ZERG_SPORECRAWLER));

  int map_width = observation->GetGameInfo().width;
  int map_height = observation->GetGameInfo().height;
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
}

void BasicSc2Bot::OnStep() {
  const ObservationInterface *observation = Observation();
  // Initialization check
    if (!initialized) {
        // Check if our starting hatchery is available
        Units own_hatcheries = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
        if (own_hatcheries.empty()) {
            return;
        }

        // Check if neutral mineral patches are available
        Units mineral_patches = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));
        if (mineral_patches.empty()) {
            return;
        }

        // Check if enemy starting locations are available
        if (observation->GetGameInfo().enemy_start_locations.empty()) {
            return;
        }

        // All necessary data is available, start onGameStart initialization in steps
        if (!initialized_mineral_patches) {
          InitializeMineralPatches();
        }
        if (initialized_mineral_patches && !initialized_hatchery) {
          InitializeStartingHatchery();
        }
        if (initialized_hatchery && !initialized_build_order) {
          InitializeBuildOrderAndScouts();
          initialized = true;
        }
    }
  // In case enemies destroy our roach warren, so we can continue attacking.
  const Units roach_warrens = observation->GetUnits(
        Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_ROACHWARREN));
  if (roach_warren_built && roach_warrens.empty()){
    tryBuild(BuildOrderItem(0, UNIT_TYPEID::ZERG_ROACHWARREN));
    std::cout << "Reattempting to build Roach Warren." << std::endl;
  }
  int current_supply = observation->GetFoodUsed();
  int supply_cap = observation->GetFoodCap();
  // Change it to build at cap for example build drones till 13

  // build next structure/unit
  if (!build_order.isEmpty()) {
    auto buildItem = build_order.peek();
    if (current_supply >= buildItem.supply) {
      if (tryBuild(buildItem)) {
        std::cout << "Building: " << UnitTypeToName(buildItem.unit_type) << std::endl;
        build_order.pop();
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
        ++built_drones;
      }
    }
    // Build roaches until army cap is x-1/x
    // Build an overlord at x-1/x army cap to gain more army space.
    // We ensure not to queue multiple overlords by checking if one is
    // already being built
    if (current_supply >= supply_cap - 3) {
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
            positionsAreClose(building.position, enemy_unit->pos, 0.5f)) {
          building_exists = true;
          building.tag = enemy_unit->tag; // Updating tag in case it's changing
          break;
        }
      }
      if (!building_exists) {
        // Add the enemy building to our list
        state.enemyBaseLocations.push_back({enemy_unit->tag, enemy_unit->pos});
        Units overlords = observation->GetUnits(
            Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
        for (size_t j = 0; j < overlords.size(); j++) {
          Actions()->UnitCommand(overlords[j], ABILITY_ID::MOVE_MOVE,
                                 state.overlord_rally_point);
        }
      }
    }
  }

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
    int random_index = GetRandomInteger(0, state.enemyBaseLocations.size() - 1);
    const GameManager::EnemyBuilding &target_building =
        state.enemyBaseLocations.at(random_index);
    launchAttack(attack_group, target_building);
    current_roach_group.clear(); // Reset the group for the next wave
  }
  patrolScouts();

  // Checking for end game scouting condition for roaches
  if (state.enemyBaseLocations.empty() && !attacking_roaches.empty()) {
    // Assign roaches to unscouted mineral patches
    Units scouting_roaches;
    for (auto tag : attacking_roaches) {
      const Unit *roach = observation->GetUnit(tag);
      if (roach && roach_scouting_assignments.find(tag) ==
                       roach_scouting_assignments.end()) {
        scouting_roaches.push_back(roach);
      }
    }

    for (const auto &roach : scouting_roaches) {
      if (!unscouted_mineral_patches.empty()) {
        Point2D target_patch = unscouted_mineral_patches.back();
        unscouted_mineral_patches.pop_back();
        Actions()->UnitCommand(roach, ABILITY_ID::MOVE_MOVE, target_patch);
        roach_scouting_assignments[roach->tag] = target_patch;
      }
    }
  } else if (!state.enemyBaseLocations.empty()) {
    // Found new enemy buildings
    if (!roach_scouting_assignments.empty()) {
      Units attacking_roaches_units;
      for (auto tag : attacking_roaches) {
        const Unit *roach = observation->GetUnit(tag);
        if (roach) {
          attacking_roaches_units.push_back(roach);
        }
      }
      Actions()->UnitCommand(attacking_roaches_units, ABILITY_ID::ATTACK,
                             state.enemyBaseLocations.at(0).position);
      // Clear scouting assignments
      roach_scouting_assignments.clear();
    }
  }

  // Queen inject on step since they don't revert back to idle after injecting
  // initially.
  HandleQueenInjects();
  // Balance drones among hatcheries
  BalanceWorkers();

  // Check Drones assigned to build structures
  for (auto it = drone_build_map.begin(); it != drone_build_map.end(); ) {
    Tag drone_tag = it->first;
    BuildOrderItem build_item = it->second.build_item;
    uint64_t assigned_game_loop = it->second.assigned_game_loop;
    uint64_t current_game_loop = observation->GetGameLoop();
    const Unit *drone = observation->GetUnit(drone_tag);

    // Don't wanna check too soon as the order may not be fully processed yet.
    if (current_game_loop - assigned_game_loop < 20) {
      ++it;
      continue;
    }
    // Checking if building is currently under construction 
    bool building_exists = false;
    Units structures = observation->GetUnits(Unit::Alliance::Self, IsUnit(build_item.unit_type));
    for (const auto &structure : structures) {
        if (structure->build_progress > 0.0f && structure->build_progress < 1.0f) {
            building_exists = true;
            break;
        }
    }

    if (building_exists) {
        std::cout << UnitTypeToName(build_item.unit_type) << " is already under construction or built." << std::endl;
        it = drone_build_map.erase(it);
        continue;
    }

    // If drone is dead, skip it. (Most likely dead because it's building the structure)
    if (!drone) {
      it = drone_build_map.erase(it);
      continue;
    }

    const sc2::UnitTypeData unit_data = observation->GetUnitTypeData().at(static_cast<uint32_t>(build_item.unit_type));
    // Check if the Drone has the correct order
    bool has_correct_order = false;
    for (const auto &order : drone->orders) {
      if (order.ability_id == unit_data.ability_id) {
        has_correct_order = true;
        break;
      }
    }

    if (!has_correct_order) {
      // Drone is not following the correct order
      std::cout << "Drone " << drone_tag << " is not building "
                << UnitTypeToName(build_item.unit_type) << ". Requeueing." << std::endl;
      build_order.push_front(build_item);
      it = drone_build_map.erase(it);
      continue;
    }
    ++it;
  }

  return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
  state.idleUnits.push_back(unit);
  switch (unit->unit_type.ToType()) {
  case UNIT_TYPEID::ZERG_DRONE: {
     // Check if the Drone was assigned to build something
        auto it = drone_build_map.find(unit->tag);
        if (it != drone_build_map.end()) {
            BuildOrderItem build_item = it->second.build_item;
            build_order.push_front(build_item);
            std::cout << "Re-queued building: " << UnitTypeToName(build_item.unit_type) << std::endl;
            
            // Check if the structure is built or under construction
            // bool structure_exists = false;
            // Units structures = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(build_item.unit_type));
            // for (const auto &structure : structures) {
            //     if (structure->build_progress > 0.0f) {
            //         structure_exists = true;
            //         break;
            //     }
            // }

            // if (!structure_exists) {
                // build_order.push_front(build_item);
                // std::cout << "Re-queued building: " << UnitTypeToName(build_item.unit_type) << std::endl;
            // }
            drone_build_map.erase(it);
        }
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
      const Unit *mineral_target = FindNearestMineralPatchForHarvest(unit->pos);
      if (!mineral_target) {
        break;
      }
      Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
    }
    break;
  }
  case UNIT_TYPEID::ZERG_OVERLORD: {
    if (scout_locations.size() > 0) {
      Actions()->UnitCommand(unit, ABILITY_ID::SMART, scout_locations.front());
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
          float distance = DistanceSquared2D(unit->pos, hatchery->pos);
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
      auto target_it = roach_attack_targets.find(unit->tag);
      if (target_it != roach_attack_targets.end()) {
        const GameManager::EnemyBuilding &target_building = target_it->second;
        const Unit *building_unit = Observation()->GetUnit(target_building.tag);
        if (!building_unit || !building_unit->is_alive) {
          // Building is destroyed or no longer exists
          // Remove it from enemyBaseLocations
          auto building_it = std::find_if(
              state.enemyBaseLocations.begin(), state.enemyBaseLocations.end(),
              [&target_building](const GameManager::EnemyBuilding &building) {
                return building.tag == target_building.tag;
              });

          if (building_it != state.enemyBaseLocations.end()) {
            state.enemyBaseLocations.erase(building_it);
          }
        }
        roach_attack_targets.erase(target_it);
      }
      // If there's a base to attack
      if (!state.enemyBaseLocations.empty()) {
        int random_index =
            GetRandomInteger(0, state.enemyBaseLocations.size() - 1);
        const Point2D &target =
            state.enemyBaseLocations.at(random_index).position;
        Actions()->UnitCommand(unit, ABILITY_ID::ATTACK,
                               target); // Attacks next target instead
                                        // of going to rally point
      } else {
        if (!unscouted_mineral_patches
                 .empty()) { // Check if there mineral patches to scoute
          // Assign the next unscouted mineral patch
          Point2D target_patch = unscouted_mineral_patches.back();
          unscouted_mineral_patches.pop_back();
          Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, target_patch);
          roach_scouting_assignments[unit->tag] = target_patch;
        } else {
          Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                                 state.rally_point);
        }
      }
    } else {
      Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, state.rally_point);
    }
  }
  default: {
    break;
  }
  }
}

void BasicSc2Bot::OnBuildingConstructionComplete(const Unit *unit) {
  // If the unit that finished constructing is an Extractor, assign 3
  // drones to the extractor.
  if (unit->unit_type == UNIT_TYPEID::ZERG_EXTRACTOR) {
    AssignDronesToExtractor(unit);
  }
  if (unit->unit_type == UNIT_TYPEID::ZERG_HATCHERY) {
    state.rally_point = unit->pos;
  }
}

void BasicSc2Bot::OnUnitDestroyed(const Unit *unit) {
  if (unit->unit_type == UNIT_TYPEID::ZERG_DRONE &&
      unit->alliance == sc2::Unit::Alliance::Self) {
      // Check if the Drone was assigned to build something
      auto it = drone_build_map.find(unit->tag);
      if (it != drone_build_map.end()) {
        BuildOrderItem build_item = it->second.build_item;
        std::cout << "Drone consumed while building: " << UnitTypeToName(build_item.unit_type) << std::endl;
        // Check if the structure is built or under construction
        bool structure_exists = false;
        Units structures = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(build_item.unit_type));
        for (const auto &structure : structures) {
            if (structure->build_progress > 0.0f) {
                structure_exists = true;
                break;
            }
        }

        if (!structure_exists) {
            build_order.push_front(build_item);
            std::cout << "Re-queued building: " << UnitTypeToName(build_item.unit_type) << std::endl;
        }
        drone_build_map.erase(it);
      }
    gas_harvesting_drones.erase(unit->tag);
  } else if (unit->unit_type == UNIT_TYPEID::ZERG_OVERLORD &&
             unit->alliance == sc2::Unit::Alliance::Self) {
    build_order.push_front(BuildOrderItem(0, UNIT_TYPEID::ZERG_OVERLORD));
  } else if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH &&
             unit->alliance == sc2::Unit::Alliance::Self) {
    attacking_roaches.erase(unit->tag); // Clean up attacking_roaches
    roach_scouting_assignments.erase(unit->tag);
  } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING &&
             unit->alliance == sc2::Unit::Alliance::Self) {
    scouts_nw.erase(unit->tag);
    scouts_ne.erase(unit->tag);
    scouts_sw.erase(unit->tag);
    scouts_se.erase(unit->tag);
    scouts.erase(unit->tag);
  } else if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH &&
             unit->alliance == sc2::Unit::Alliance::Self) {
    attacking_roaches.erase(unit->tag);
    roach_attack_targets.erase(unit->tag);
    roach_scouting_assignments.erase(unit->tag);
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
          state.enemyBaseLocations.begin(), state.enemyBaseLocations.end(),
          [unit, this](const GameManager::EnemyBuilding &building) {
            return (positionsAreClose(building.position, unit->pos, 1.5f) ||
                    (building.tag == unit->tag));
          });
      if (it != state.enemyBaseLocations.end()) {
        state.enemyBaseLocations.erase(it, state.enemyBaseLocations.end());
      }
    }
  }
}

void BasicSc2Bot::OnUnitCreated(const Unit *unit) {
  switch (unit->unit_type.ToType()) {
  case UNIT_TYPEID::ZERG_ROACH: {
    // Add Roach to the current group
    current_roach_group.push_back(unit->tag);
    // Move Roach to the rally point
    Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, state.rally_point);
    break;
  }
  case sc2::UNIT_TYPEID::ZERG_ZERGLING: {
    std::vector<Point2D> *scout_points;
    std::set<Tag> *scouts_at_quad;
    bool added_scout = false;
    if (scouts_nw.size() < MAX_SCOUTS_PER_QUAD) {
      scout_points = &scout_loc_north_west;
      scouts_at_quad = &scouts_nw;
      added_scout = true;
    } else if (scouts_ne.size() < MAX_SCOUTS_PER_QUAD) {
      scout_points = &scout_loc_north_east;
      scouts_at_quad = &scouts_ne;
      added_scout = true;
    } else if (scouts_sw.size() < MAX_SCOUTS_PER_QUAD) {
      scout_points = &scout_loc_south_west;
      scouts_at_quad = &scouts_sw;
      added_scout = true;
    } else if (scouts_se.size() < MAX_SCOUTS_PER_QUAD) {
      scout_points = &scout_loc_south_east;
      scouts_at_quad = &scouts_se;
      added_scout = true;
    }
    if (added_scout) {
      // get scout info
      // add index of scout
      // add it to scout set
      int index = GetRandomInteger(0, scout_points->size() - 1);
      scouts.insert(std::pair<Tag, Scout>(unit->tag, Scout(unit->tag, index)));
      scouts_at_quad->insert(unit->tag);
      Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE,
                             scout_points->at(index));
    }
    break;
  }
  default: {
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
    if (gas_harvesting_drones.find(drone->tag) != gas_harvesting_drones.end()) {
      continue;
    }

    // Skip drones assigned to building
    if (drone_build_map.find(drone->tag) != drone_build_map.end()) {
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

  for (float radius = search_radius; radius <= search_radius + 5.0f;
       radius += 1.0f) {
    for (float angle = 0.0f; angle < 360.0f; angle += angle_step) {
      float radians = angle * 3.14159f / 180.0f;
      Point2D build_position =
          Point2D(target_position.x + radius * cos(radians),
                  target_position.y + radius * sin(radians));

      // Check if the position is valid for building a Hatchery
      if (Query()->Placement(ABILITY_ID::BUILD_HATCHERY, build_position)) {
        return build_position;
      }
    }
  }

  // No valid position found
  return Point2D(0.0f, 0.0f);
}
const Unit *BasicSc2Bot::findIdleLarva() {
  for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
    if (unit->unit_type == UNIT_TYPEID::ZERG_LARVA && unit->orders.empty()) {
      return unit;
    }
  }
  return nullptr;
}
const Unit *BasicSc2Bot::findIdleDrone() {
  for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
    if (unit->unit_type == UNIT_TYPEID::ZERG_DRONE && unit->orders.empty()) {
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
    const Unit *initial_hatchery = observation->GetUnit(initial_hatchery_tag);
    if (initial_hatchery && initial_hatchery->build_progress == 1.0f) {
      if (observation->GetMinerals() >= 150 &&
          observation->GetVespene() >= 100) {
        Actions()->UnitCommand(initial_hatchery, ABILITY_ID::MORPH_LAIR);
        return true;
      } else {
      }
    } else {
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
          observation->GetVespene() >= 25 && current_supply < supply_cap) {
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
      // Check that we have enough supply to build zergling.
      int current_supply = observation->GetFoodUsed();
      int supply_cap = observation->GetFoodCap();
      if (current_supply + 1 > supply_cap) {
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
      Units hatcheries = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
      for (const auto &hatchery : observation->GetUnits(
               Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))) {
        if (hatchery->tag == first_queen_hatchery) {
          continue;
        }
        if (hatchery->build_progress == 1.0f && hatchery->orders.empty() &&
            observation->GetMinerals() >= 150) {
          bool queen_exists = false;
          for (const auto &unit : observation->GetUnits(Unit::Alliance::Self)) {
                if (unit->unit_type == UNIT_TYPEID::ZERG_QUEEN) {
                    if (DistanceSquared2D(unit->pos, hatchery->pos) < 10.0f) {
                        queen_exists = true;
                        break;
                    }
                }
            }
          if (!queen_exists) {
            Actions()->UnitCommand(hatchery, ABILITY_ID::TRAIN_QUEEN);
            first_queen_hatchery = hatchery->tag;
            std::cout << "Building at Hatchery with Tag: " << hatchery->tag << std::endl;
            return true;
          } else {
            continue;
          }
          
        }
      }
      break;
    }

    // if its a hatchery we get the nearest mineral location that we
    // havent visited
    case UNIT_TYPEID::ZERG_HATCHERY: {
      const Unit *drone = findAvailableDrone();
      if (drone && observation->GetMinerals() >= 300) {
        // locate the next nearest mineral group
        const Unit *mineral_cluster_a = FindNearestMineralPatch(drone->pos);
        const Unit *mineral_cluster_b =
            findNextNearestMineralGroup(mineral_cluster_a);
        // get map center
        Point2D map_center = getMapCenter();
        // get vector from cluster center to map center, normalize
        // into a direction vector
        Point2D direction_vector(
            getDirectionVector(mineral_cluster_b->pos, map_center));
        // create point for hatchery
        Point2D hatchery_location(
            mineral_cluster_b->pos.x + (direction_vector.x * 3.0f),
            mineral_cluster_b->pos.y + (direction_vector.y * 3.0f));
        // find build position
        Point2D build_position =
            findBuildPositionNearMineral(hatchery_location);
        if (build_position.x != 0.0f || build_position.y != 0.0f) {
          Actions()->UnitCommand(drone, ABILITY_ID::BUILD_HATCHERY,
                                 build_position);
          drone_build_map[drone->tag] = DroneBuildTask(buildItem, observation->GetGameLoop());
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
          Units hatcheries =
              observation->GetUnits(Unit::Alliance::Self, IsTownHall());
          for (const auto &hatchery : hatcheries) {
            if (hatchery->build_progress == 1.0f &&
                hatchery->tag != initial_hatchery_tag) {
              target_hatchery = hatchery;
              break;
            }
          }
        } else {
          // If building first 2 extractors, we can use the
          // position of the initial hatchery
          target_hatchery = observation->GetUnit(initial_hatchery_tag);
        }
        if (target_hatchery) {
          const Unit *nearest_vespene =
              FindNearestVespenePatch(target_hatchery->pos);
          if (nearest_vespene) {
            Actions()->UnitCommand(drone, ABILITY_ID::BUILD_EXTRACTOR,
                                   nearest_vespene);
            drone_build_map[drone->tag] = DroneBuildTask(buildItem, observation->GetGameLoop());
            ++built_extractors;
            return true;
          }
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_SPAWNINGPOOL: {
      const Unit *drone = findAvailableDrone();
      if (drone && observation->GetMinerals() >= 200) {
        Point2D build_position =
            FindPlacementLocation(ABILITY_ID::BUILD_SPAWNINGPOOL, drone->pos, drone);
        if (build_position != Point2D(0.0f, 0.0f)) {
          Actions()->UnitCommand(drone, ABILITY_ID::BUILD_SPAWNINGPOOL,
                               build_position);
          drone_build_map[drone->tag] = DroneBuildTask(buildItem, observation->GetGameLoop());
          return true;
        } else {
          return false;
        }
        
      }
      break;
    }
    case UNIT_TYPEID::ZERG_ROACHWARREN: {
      const Unit *drone = findAvailableDrone();
      if (drone && observation->GetMinerals() >= 150) {
        AbilityID build_ability = ABILITY_ID::BUILD_ROACHWARREN;
        Point2D build_position =
            FindPlacementLocation(build_ability, drone->pos, drone);
        if (build_position != Point2D(0.0f, 0.0f)) {
          Actions()->UnitCommand(drone, build_ability, build_position);
          roach_warren_built = true; // Tag to retry building if it's not building properly (Issue specific to server)
          drone_build_map[drone->tag] = DroneBuildTask(buildItem, observation->GetGameLoop());
          return true;
        } else {
          std::cout << "Cant find a build location for roach warren" << std::endl;
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_SPORECRAWLER: {
      const Unit *drone = findAvailableDrone();
      if (drone && observation->GetMinerals() >= 75) {
        AbilityID build_ability = ABILITY_ID::BUILD_SPORECRAWLER;
        Point2D build_position =
            FindPlacementLocation(build_ability, drone->pos, drone);
        if (build_position != Point2D(0.0f, 0.0f)) {
          Actions()->UnitCommand(drone, build_ability, build_position);
          drone_build_map[drone->tag] = DroneBuildTask(buildItem, observation->GetGameLoop());
          std::cout << "Issued build command for: " << UnitTypeToName(buildItem.unit_type)
                      << " at position (" << build_position.x << ", " << build_position.y << ")" << std::endl;
          return true;
        }
      }
      break;
    }

    default:
      const Unit *drone = findAvailableDrone();
      if (drone) {
        Point2D build_position = Point2D(drone->pos.x - 50, drone->pos.y - 100);

        Actions()->UnitCommand(drone, buildItem.ability, build_position);
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

void BasicSc2Bot::launchAttack(
    const Units &attack_group,
    const GameManager::EnemyBuilding &target_building) {
  Actions()->UnitCommand(
      attack_group, ABILITY_ID::ATTACK,
      target_building.position); // Send group of roaches to attack
  // Add roach tags to attacking_roaches set
  for (const auto &unit : attack_group) {
    attacking_roaches.insert(unit->tag);
    roach_attack_targets[unit->tag] =
        target_building; // Map roach to the building it's attacking
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
    const std::vector<Point2D> *points;
    if (scouts_nw.find(scout.unit_tag) != scouts_nw.end()) {
      points = &scout_loc_north_west;
    } else if (scouts_ne.find(scout.unit_tag) != scouts_ne.end()) {
      points = &scout_loc_north_east;
    } else if (scouts_sw.find(scout.unit_tag) != scouts_sw.end()) {
      points = &scout_loc_south_west;
    } else if (scouts_se.find(scout.unit_tag) != scouts_se.end()) {
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
   * finds and returns a mineral patch in the next nearest group of
   * mineral patches
   */
  const Unit *mineral_loc_b = FindNearestMineralPatch(mineral_loc_a->pos);
  for (int i = 0; i < 10; i++) {
    // compare differences
    if (getVectorDifferenceMagnitude(mineral_loc_a->pos, mineral_loc_b->pos) >
        20.00) {
      // mineral_loc_b is further from previous minerals by a factor
      // of 10
      return mineral_loc_b;
    }
    // get next mineral pairs
    mineral_loc_a = mineral_loc_b;
    mineral_loc_b = FindNearestMineralPatch(mineral_loc_b->pos);
  }
  return mineral_loc_b; // in case of next group not found within 10
                        // minerals, return 10th mineral patch
}
double BasicSc2Bot::getVectorDifferenceMagnitude(Point2D vec_a, Point2D vec_b) {
  /**
   * calculates and returns the magnitude of the difference between two
   * points
   */
  Point2D difference_vector(vec_a.x - vec_b.x, vec_a.y - vec_b.y);
  double diff = sqrt(difference_vector.x * difference_vector.x +
                     difference_vector.y * difference_vector.y);
  return diff;
}
Point2D BasicSc2Bot::getDirectionVector(Point2D vec_a, Point2D vec_b) {
  /**
   * calculates and returns the direction vector of the line from point a
   * to point b
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
  if (abs(pos.x - rally.x) < range && abs(pos.y - rally.y) < range) {
    return true;
  } else {
    return false;
  }
}

// Samples from a circle around the center point to check if there is creep every where
bool BasicSc2Bot::IsAreaOnCreep(const Point2D &center, float building_radius) {
    int num_samples = 20;
    float angle_step = 2 * M_PI / num_samples;

    for (int i = 0; i < num_samples; ++i) {
        float angle = i * angle_step;
        float x_offset = building_radius * cos(angle);
        float y_offset = building_radius * sin(angle);
        Point2D sample_point = center + Point2D(x_offset, y_offset);

        if (!Observation()->HasCreep(sample_point)) {
            return false; // Found a point without creep
        }
    }
    return true; // All sampled points have creep
}

Point2D BasicSc2Bot::FindPlacementLocation(AbilityID ability,
                                           const Point2D &near_point,
                                           const Unit* unit) {
    float building_radius = 1.5f;

    for (int i = 0; i < 30; ++i) {
        float rx = GetRandomScalar() * 10.0f - 5.0f;
        float ry = GetRandomScalar() * 10.0f - 5.0f;
        Point2D test_point = near_point + Point2D(rx, ry);

        // Check pathing
        float path_distance = Query()->PathingDistance(near_point, test_point);
        if (path_distance < 0.0f) {
            continue; // Cannot reach the location
        }

        // Check placement and creep coverage
        if (Query()->Placement(ability, test_point) &&
            IsAreaOnCreep(test_point, building_radius)) {
            return test_point;
        }
    }
    return Point2D(0.0f, 0.0f); // Indicates failure
}

// To be called onStep() to make sure queens are injecting as soon as they
// can
void BasicSc2Bot::HandleQueenInjects() {
  const ObservationInterface *observation = Observation();
  Units queens = observation->GetUnits(Unit::Alliance::Self,
                                       IsUnit(UNIT_TYPEID::ZERG_QUEEN));
  Units hatcheries = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
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
            float distance = DistanceSquared2D(queen->pos, hatchery->pos);
            if (distance < min_distance) {
              min_distance = distance;
              closest_hatchery = hatchery;
            }
          }
        }
        // Inject closest hatchery
        if (closest_hatchery) {
          Actions()->UnitCommand(queen, ABILITY_ID::EFFECT_INJECTLARVA,
                                 closest_hatchery);
        }
      }
    }
  }
}

// Used in OnBuildingConstructionComplete to assign 3 drones to an extractor
// as soon as it's built
void BasicSc2Bot::AssignDronesToExtractor(const Unit *extractor) {
  std::set<Tag>
      assigned_drones; // Set to track which drones have already been
                       // assigned to not duplicate call the same drone.
  int drone_count = 0;
  while (drone_count < 3) {
    const Unit *drone = findAvailableDrone();
    if (!drone) {
      break;
    }
    // Checks if drone is a valid unit and whether the drone's tag is
    // already in the set of assigned drones.
    if (drone && (assigned_drones.find(drone->tag) == assigned_drones.end())) {
      Actions()->UnitCommand(drone, ABILITY_ID::HARVEST_GATHER, extractor);
      assigned_drones.insert(drone->tag);
      // Adding Tag to global set so findAvailableDrone() doesn't grab
      // gas harvesting drones.
      gas_harvesting_drones.insert(drone->tag);
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
  Units hatcheries = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
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
  // Exit loop if there's no over or under saturated bases. If one is
  // empty, there's no reassigning to be done.
  if (overSaturatedBases.empty() || underSaturatedBases.empty()) {
    return;
  }
  // Get correct amount of additional drones from over saturated base to
  // send to under saturated base
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
        dronesToTransfer.push_back(drone); // If it's gathering minerals and
                                           // near the hatchery, transfer it
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
        }
      }
    }
  }
}