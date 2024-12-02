#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "AttackBaseQueue.h"
#include "BuildQueue.h"
#include "GameManager.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2utils/sc2_manage_process.h"
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_unit_filters.h>
#include <set>
#include <unordered_set>
#include <vector>


using namespace sc2;

class BasicSc2Bot : public Agent {
public:
  BasicSc2Bot(int group_size = 8, int additional_drones = 11)
      : group_size(group_size), additional_drones(additional_drones){};
  virtual void OnGameStart();
  virtual void OnStep();
  virtual void OnUnitIdle(const Unit *unit) final;
  virtual void OnBuildingConstructionComplete(const sc2::Unit *unit);
  virtual void OnUnitDestroyed(const Unit *unit);
  virtual void OnUnitCreated(const Unit *unit);
  virtual void OnGameEnd();

private:
  const Unit *FindNearestMineralPatch(const Point2D &);
  const Unit *FindNearestMineralPatchForHarvest(const Point2D &);
  const Unit *FindNearestVespenePatch(const Point2D &);
  const Point2D getValidRallyPoint(const Point2D &base_position,
                                   QueryInterface *query,
                                   float max_radius = 15.0f, float step = 2.0f);
  Point2D FindPlacementLocation(AbilityID ability, const Point2D &near_point);
  const Unit *findIdleLarva();
  const Unit *findIdleDrone();
  const Unit *findAvailableDrone();
  const Unit *findAvailableLarva();
  Point2D findBuildPositionNearMineral(const Point2D &);
  void HandleQueenInjects();
  void AssignDronesToExtractor(const Unit *extractor);
  bool tryBuild(struct BuildOrderItem);
  bool isArmyReady();
  bool inRallyRange(const Point2D &, const Point2D &, float);
  void launchAttack(const Units &attack_group,
                    const GameManager::EnemyBuilding &target_building);
  void BalanceWorkers();
  Point2D getMapCenter() const;
  Point2D getDirectionVector(const Point2D, const Point2D);
  double getVectorDifferenceMagnitude(const Point2D, const Point2D);
  const Unit *findNextNearestMineralGroup(const Unit *);
  bool positionsAreClose(const Point2D &a, const Point2D &b, float tolerance);
  std::vector<Point2D> scout_loc_north_east, scout_loc_north_west,
      scout_loc_south_east, scout_loc_south_west; // scouting locations
  struct Scout {
    Tag unit_tag; // Tag of the Zergling assigned as the scout
    size_t
        current_target_idx; // Index of the current resource point being scouted
    Scout(Tag t, size_t curr) : unit_tag(t), current_target_idx(curr) {}
  };
  std::set<Tag> scouts_ne, scouts_nw, scouts_se,
      scouts_sw; // scouts for each location
  std::map<Tag, Scout> scouts;
  void patrolScouts();
  BuildQueue build_order; // Queue that holds BuildOrderItems
  std::vector<Point2D>
      scout_locations; // Vector containing locations we need to scout
  std::set<const Unit *> mineral_locations;
  std::set<const Unit *> vespene_locations;
  std::set<Tag> gas_harvesting_drones;
  Tag initial_hatchery_tag; // Add this line
  AttackBaseQueue
      enemy_bases; // Queue containing locations we identify as enemy bases
  int group_size;  // Number of roaches to send per wave
  std::vector<Tag>
      current_roach_group; // Roaches in the current group to be sent to attack
                           // once vector.size() == group_size
  // Contains the Tags/ids of roaches that have already been sent to attack
  // so we can reassign them to a new building to attack once they're done
  // attacking the one initially assigned.
  std::set<Tag> attacking_roaches;
  int additional_drones; // Number of drones we want to build after the build
                         // order queue is done
  int built_drones =
      0; // Number of drones already built after build order queue is done.
         // Stops building drones when built_drones == additional_drones.
  int built_extractors = 0;
  std::vector<Point2D>
      unscouted_mineral_patches; // Tracks which mineral patches the roaches
                                 // haven't scouted yet.
  std::map<Tag, Point2D>
      roach_scouting_assignments; // Tracks which roach is scouting which 2D
                                  // point on the map.
  std::unordered_map<Tag, GameManager::EnemyBuilding>
      roach_attack_targets; // Map from Roach Tag to Building Tag
};

#endif