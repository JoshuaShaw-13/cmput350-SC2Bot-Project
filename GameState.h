#ifndef GAME_STATE_
#define GAME_STATE_

#include <sc2api/sc2_unit_filters.h>
#include <vector>

using namespace sc2;
// enum for current policy (saving?, building?, rushing?)
enum Policy { BUILDING, ATTACKING, SAVING };

class GameState {
public:
  GameState();
  ~GameState();
  GameState(const GameState &q);            // copy constructor
  GameState &operator=(const GameState &q); // assignment operator

  // arrays for idle units (group in some way)
  std::vector<const Unit *> idleUnits;

  // array of attackable enemy bases (logs unit that represents a base, like a
  // command center)
  std::vector<const Unit *> enemyBaseLocations;
  int curr_policy;
};

#endif