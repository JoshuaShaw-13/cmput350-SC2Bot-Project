#include "BasicSc2Bot.h"
#include "GameManager.h"
#include <cmath>
#include <iostream>
#include <ostream>
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_unit_filters.h>
#define M_PI 3.14

using namespace sc2;

GameManager state;

void BasicSc2Bot::OnGameStart() {
  const ObservationInterface *observation = Observation();
  scout_locations = observation->GetGameInfo().enemy_start_locations;

  // Build drones if not at the given army cap for next item
  build_order.push(BuildOrderItem(
      13, UNIT_TYPEID::ZERG_OVERLORD)); // At 13 cap, build an Overlord
  build_order.push(BuildOrderItem(
      16, UNIT_TYPEID::ZERG_HATCHERY)); // At 16 cap, build a Hatchery
  build_order.push(BuildOrderItem(
      18, UNIT_TYPEID::ZERG_EXTRACTOR)); // At 18 cap, build an Extractor
  build_order.push(BuildOrderItem(
      19,
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
  state.rally_point =
      getValidRallyPoint(observation->GetStartLocation(), Query());

  // find overlord rally location depending on base start location
  const Unit *start_base =
      observation
          ->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))
          .front();
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

  // Send the first Overlord to scout one of the scout_locations and remove it
  // from further scouting consideration
  Units overlords = observation->GetUnits(Unit::Alliance::Self,
                                          IsUnit(UNIT_TYPEID::ZERG_OVERLORD));
  if (!overlords.empty() && !scout_locations.empty()) {
    const Unit *first_overlord = overlords.front();
    Actions()->UnitCommand(first_overlord, ABILITY_ID::SMART,
                           scout_locations.front());
    scout_locations.erase(scout_locations.begin());
  }
}

void BasicSc2Bot::OnStep() {
  const ObservationInterface *observation = Observation();
  int current_supply = observation->GetFoodUsed();
  // Change it to build at cap for example build drones till 13

  // build next structure/unit
  if (!build_order.isEmpty()) {
    auto buildItem = build_order.peek();
    int count;
    if (current_supply >= buildItem.supply) {
      if (tryBuild(buildItem)) {
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
  }

  // build overseers when we are nearing troop capacity
  // buildOverseers();

  // check if we have enough resources to expand to a new base
  //  this would be in the manager

  // Attack once we have an optimal army build
  // use the attack base queue
  if (isArmyReady() && !launching_attack) {
    if (!enemy_bases.isEmpty()) {
      launchAttack(enemy_bases.peek()->loc);
    }
  }

  return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
  state.idleUnits.push_back(unit);
  switch (unit->unit_type.ToType()) {
  case UNIT_TYPEID::ZERG_DRONE: {
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
      const Unit *mineral_target = FindNearestMineralPatch(unit->pos);
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
    if (state.scouts.front() == unit && scout_locations.size() > 0) {
      Actions()->UnitCommand(unit, ABILITY_ID::SMART, scout_locations.front());
      scout_locations.erase(scout_locations.begin());
      state.scouts.push_back(unit);
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
            min_distance = distance;
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
const Unit *BasicSc2Bot::FindNearestVespenePatch(const Point2D &start) {
  Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
  float distance = std::numeric_limits<float>::max();
  const Unit *target = nullptr;
  for (const auto &u : units) {
    if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER &&
        vespene_locations.find(u) == vespene_locations.end()) {
      float d = DistanceSquared2D(u->pos, start);
      if (d < distance) {
        distance = d;
        target = u;
      }
    }
  }
  vespene_locations.insert(target);
  return target;
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
const Unit *BasicSc2Bot::findAvailableDrone() {
  Units drones = Observation()->GetUnits(Unit::Alliance::Self,
                                         IsUnit(UNIT_TYPEID::ZERG_DRONE));

  // Prioritize idle Drones
  for (const auto &drone : drones) {
    if (drone->orders.empty()) {
      return drone;
    }
  }

  // If no idle Drones, find one that's harvesting minerals
  for (const auto &drone : drones) {
    if (!drone->orders.empty()) {
      AbilityID current_ability = drone->orders.front().ability_id;
      if (current_ability == ABILITY_ID::HARVEST_GATHER ||
          current_ability == ABILITY_ID::HARVEST_RETURN) {
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
      build_radius + 15.0f;       // Add extra space for clearance
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
      if (Query()->Placement(ABILITY_ID::BUILD_HATCHERY, build_position)) {
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
    for (float y = -search_radius; y <= search_radius; y += search_increment) {
      Point2D test_location = start + Point2D(x, y);

      // Check if the position is buildable for the Spawning Pool
      if (Query()->Placement(ABILITY_ID::BUILD_SPAWNINGPOOL, test_location)) {
        return test_location;
      }
    }
  }
  return Point2D(0, 0);
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
    case UNIT_TYPEID::ZERG_ROACH:
      larva = findIdleLarva();
      if (larva && observation->GetMinerals() >= 75 &&
          observation->GetVespene() >= 25) {
        Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ROACH);
        return true;
      }
      break;

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
      for (const auto &hatchery : observation->GetUnits(
               Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))) {
        if (hatchery->orders.empty() && observation->GetMinerals() >= 150) {
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
        const Unit *mineral_cluster_a = FindNearestMineralPatch(drone->pos);
        const Unit *mineral_cluster_b =
            findNextNearestMineralGroup(mineral_cluster_a);
        std::cout << "located mineral cluster: " << mineral_cluster_b->pos.x
                  << " , " << mineral_cluster_b->pos.y << std::endl;
        // get map center
        Point2D map_center = getMapCenter();
        // get vector from cluster center to map center, normalize into a
        // direction vector
        Point2D direction_vector(
            getDirectionVector(mineral_cluster_b->pos, map_center));
        // create point for hatchery
        std::cout << "direction vector: " << direction_vector.x << " , "
                  << direction_vector.y << std::endl;
        Point2D hatchery_location(
            mineral_cluster_b->pos.x + (direction_vector.x * 15.0f),
            mineral_cluster_b->pos.y + (direction_vector.y * 15.0f));
        std::cout << "calculated hatchery point: " << hatchery_location.x
                  << " , " << hatchery_location.y << std::endl;
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
        const Unit *nearest_vespene_loc = FindNearestVespenePatch(drone->pos);

        Actions()->UnitCommand(drone, ABILITY_ID::BUILD_EXTRACTOR,
                               nearest_vespene_loc);
        return true;
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

    default:
      const Unit *drone = findAvailableDrone();
      if (drone) {
        float rx = GetRandomScalar();
        float ry = GetRandomScalar();
        Point2D build_position = Point2D(drone->pos.x - 50, drone->pos.y - 100);

        Actions()->UnitCommand(drone, buildItem.ability, build_position);
        return true;
      }
    }
    return false;
  }
  // must be an upgrade
  if (buildItem.ability != ABILITY_ID::INVALID) {
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
  const int optRoach = 0;
  const int optZergling = 8;
  for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
    if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
      roach_count++;
    } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING) {
      zergling_count++;
    }
  }
  return roach_count >= optRoach && zergling_count >= optZergling;
}
void BasicSc2Bot::launchAttack(const Point2D &target) {
  for (const auto &unit : Observation()->GetUnits(Unit::Alliance::Self)) {
    if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING ||
        unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
      Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
      launching_attack = true;
    }
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
    if (getVectorDifferenceMagnitude(mineral_loc_a->pos, mineral_loc_b->pos) >
        20.00) {
      // mineral_loc_b is further from previous minerals by a factor of 10
      return mineral_loc_b;
    }
    // get next mineral pairs
    mineral_loc_a = mineral_loc_b;
    mineral_loc_b = FindNearestVespenePatch(mineral_loc_b->pos);
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
    if (Query()->Placement(ability, test_point)) {
      return test_point;
    }
  }
}
