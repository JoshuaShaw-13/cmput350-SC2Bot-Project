#include "GameManager.h"

using namespace sc2;

GameManager::GameManager() : curr_policy(Policy::BUILDING){};

GameManager::~GameManager(){};

GameManager::GameManager(const GameManager &gm) : curr_policy(gm.curr_policy) {
  idleUnits = gm.idleUnits;
  enemyBaseLocations = gm.enemyBaseLocations;
  scouts = gm.scouts;
  rally_point = gm.rally_point;
  overlord_rally_point = gm.overlord_rally_point;
  scouting_location = gm.scouting_location;
};

GameManager &GameManager::operator=(const GameManager &gm) {
  if (&gm != this) {
    curr_policy = gm.curr_policy;
    idleUnits = gm.idleUnits;
    enemyBaseLocations = gm.enemyBaseLocations;
    scouts = gm.scouts;
    rally_point = gm.rally_point;
    overlord_rally_point = gm.overlord_rally_point;
    scouting_location = gm.scouting_location;
  }
  return *this;
};