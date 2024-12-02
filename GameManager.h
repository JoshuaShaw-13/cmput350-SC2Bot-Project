#ifndef GAME_MANAGER_
#define GAME_MANAGER_

#include <sc2api/sc2_unit_filters.h>
#include <vector>

using namespace sc2;

class GameManager {
public:
  GameManager();
  ~GameManager();
  GameManager(const GameManager &q);            // copy constructor
  GameManager &operator=(const GameManager &q); // assignment operator

  // arrays for idle units (group in some way)
  std::vector<const Unit *> idleUnits;

  // Enemy base structs that hold the tag (id) as well as location of building
  struct EnemyBuilding {
    Tag tag;
    Point2D position;
  };

  // array of attackable enemy bases (logs unit that represents a base, like a
  // command center)
  std::vector<EnemyBuilding> enemyBaseLocations;

  // location points
  Point2D rally_point;
  Point2D overlord_rally_point;
};

#endif