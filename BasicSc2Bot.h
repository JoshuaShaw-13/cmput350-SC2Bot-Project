#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

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
};

#endif