#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2utils/sc2_manage_process.h"
#include <iostream>
#include <sc2api/sc2_unit_filters.h>

#include "GameState.h"

using namespace sc2;

class BasicSc2Bot : public Agent {
public:
  virtual void OnGameStart();
  virtual void OnStep();

  virtual void OnUnitIdle(const Unit *unit) final;

private:
  const Unit *FindNearestMineralPatch(const Point2D &start);
};

#endif