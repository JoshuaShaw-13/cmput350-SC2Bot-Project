#include "GameManager.h"

using namespace sc2;

GameManager::GameManager(){};

GameManager::~GameManager(){};

GameManager::GameManager(const GameManager &gm) {
  idleUnits = gm.idleUnits;
  enemyBaseLocations = gm.enemyBaseLocations;
  rally_point = gm.rally_point;
  overlord_rally_point = gm.overlord_rally_point;
};

GameManager &GameManager::operator=(const GameManager &gm) {
  if (&gm != this) {
    idleUnits = gm.idleUnits;
    enemyBaseLocations = gm.enemyBaseLocations;
    rally_point = gm.rally_point;
    overlord_rally_point = gm.overlord_rally_point;
  }
  return *this;
};