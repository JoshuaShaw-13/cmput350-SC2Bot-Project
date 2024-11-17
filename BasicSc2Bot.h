#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "AttackBaseQueue.h"
#include "BuildQueue.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2utils/sc2_manage_process.h"
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_unit_filters.h>
#include <set>
#include <vector>


using namespace sc2;

class BasicSc2Bot : public Agent {
public:
  virtual void OnGameStart();
  virtual void OnStep();

  virtual void OnUnitIdle(const Unit *unit) final;

private:
  const Unit *FindNearestMineralPatch(const Point2D &);
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
  Point2D findBuildPosition(const Point2D &);
  bool tryBuild(struct BuildOrderItem);
  bool isArmyReady();
  bool inRallyRange(const Point2D &, const Point2D &, float);
  void launchAttack(const Point2D &);
  BuildQueue build_order; // Queue that holds BuildOrderItems
  std::vector<Point2D>
      scout_locations; // Vector containing locations we need to scout
  std::set<const Unit *> mineral_locations;
  std::set<const Unit *> vespene_locations;
  bool launching_attack = false;
  AttackBaseQueue
      enemy_bases; // Queue containing locations we identify as enemy bases
};

#endif