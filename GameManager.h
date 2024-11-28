#ifndef GAME_MANAGER_
#define GAME_MANAGER_

#include <algorithm>
#include <sc2api/sc2_unit_filters.h>
#include <vector>


using namespace sc2;
// enum for current policy (saving?, building?, rushing?)
enum Policy { BUILDING, ATTACKING, SAVING };

class GameManager {
public:
  GameManager();
  ~GameManager();
  GameManager(const GameManager &q);            // copy constructor
  GameManager &operator=(const GameManager &q); // assignment operator

  // arrays for idle units (group in some way)
  std::vector<const Unit *> idleUnits;

  // array of attackable enemy bases (logs unit that represents a base, like a
  // command center)
  std::vector<Point2D> enemyBaseLocations;

  // array of scouts
  std::vector<const Unit *> scouts;

  // current policy from Policy enum
  int curr_policy;

  // location points
  Point2D rally_point;
  Point2D overlord_rally_point;
  Point2D scouting_location;
};

#endif