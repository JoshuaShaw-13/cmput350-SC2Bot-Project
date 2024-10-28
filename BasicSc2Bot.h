#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_


#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include <vector>
#include "Classes/AttackBaseQueue.h"
#include "Classes/BuildQueue.h"
#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_unit_filters.h>

using namespace sc2;

class BasicSc2Bot : public Agent {
public:
  virtual void OnGameStart();
  virtual void OnStep();

  virtual void OnUnitIdle(const Unit *unit) final;

private:
  const Unit *FindNearestMineralPatch(const Point2D &start);
  const Unit *findIdleLarva();
  const Unit *findIdleDrone();
  bool tryBuild(struct BuildOrderItem);
  bool isArmyReady();
  void launchAttack( const Point2D& target);
  BuildQueue build_order; // Queue that holds BuildOrderItems
  std::vector<Point2D> scout_locations; // Vector containing locations we need to scout
  AttackBaseQueue enemy_bases; // Queue containing locations we identify as enemy bases
};

#endif