#ifndef GAME_STATE_
#define GAME_STATE_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2utils/sc2_manage_process.h"
#include <sc2api/sc2_unit_filters.h>
#include <vector>

using namespace sc2;
// enum for current policy (saving?, building?, rushing?)
enum Policy { BUILDING, ATTACKING, SAVING };

struct GameState {
  GameState();
  ~GameState();

  // arrays for idle units (group in some way)
  std::vector<const Unit *> idleUnits;

  // array of attackable enemy bases (logs unit that represents a base, like a
  // command center)
  std::vector<const Unit *> enemyBaseLocations;
  int curr_policy;
};

#endif