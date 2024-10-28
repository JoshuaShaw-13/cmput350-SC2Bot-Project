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

class BasicSc2Bot : public sc2::Agent {
public:
	virtual void OnGameStart();
	virtual void OnStep();

private:
	// Temporary struct to hold build order
    struct BuildOrderItem {
        int supply;                   // Supply count at which to build the gien unit/ability (0 if ASAP)
        sc2::UNIT_TYPEID unit_type;   // Unit or structure to build
        sc2::ABILITY_ID ability;      // Ability to use (for upgrades or morphs)
        bool is_unit;                 // True if unit_type is valid, false if ability is valid

        // Constructor for units and structures
        BuildOrderItem(int s, sc2::UNIT_TYPEID u)
            : supply(s), unit_type(u), ability(sc2::ABILITY_ID::INVALID), is_unit(true) {}

        // Constructor for abilities
        BuildOrderItem(int s, sc2::ABILITY_ID a)
            : supply(s), unit_type(sc2::UNIT_TYPEID::INVALID), ability(a), is_unit(false) {}
    };
    BuildQueue build_order; // Queue that holds BuildOrderItems
    std::vector<sc2::Point3D> scout_locations; // Vector containing locations we need to scout
    AttackBaseQueue enemy_bases; // Queue containing locations we identify as enemy bases

     const Unit *FindNearestMineralPatch(const Point2D &start);
};

#endif