/**
 * Specialized Queue encapsulation for tracking the build order
 */
#ifndef BUILDQUEUE_H
#define BUILDQUEUE_H

#include <queue>
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class BuildQueue{
    public:
        BuildQueue(){};//constructor
        ~BuildQueue(){};//deconstructor
        BuildQueue(const BuildQueue &q){};//copy constructor
        BuildQueue &operator=(const BuildQueue &q){};//assignment operator

        //getter(s)
        std::queue<BuildOrderItem*>* getQueue() const{}

        //methods
        bool isEmpty() const{/** returns true if the queue is empty */ }
        BuildOrderItem* peek() const{/** returns a pointer to the next building in the queue */}
        BuildOrderItem* pop() {/** removes the next building in the queue, and returns a pointer to that building */};
        void push(const BuildOrderItem &BuildOrderItem){/**  adds the given BuildOrderItem to the queue*/}

    private:
    //data
    std::queue<BuildOrderItem*>* data;

};

struct BuildOrderItem{
    /** struct for holding BuildOrderItem information */
    int supply;                   // Supply count at which to build the gien BuildOrderItem/ability (0 if ASAP)
    sc2::UNIT_TYPEID unit_type;   // BuildOrderItem or structure to build
    sc2::ABILITY_ID ability;      // Ability to use (for upgrades or morphs)
    bool is_unit;                 // True if unit_type is valid, false if ability is valid

    // Constructor for units and structures
    BuildOrderItem(int s, sc2::UNIT_TYPEID u)
        : supply(s), unit_type(u), ability(sc2::ABILITY_ID::INVALID), is_unit(true) {}

    // Constructor for abilities
    BuildOrderItem(int s, sc2::ABILITY_ID a)
        : supply(s), unit_type(sc2::UNIT_TYPEID::INVALID), ability(a), is_unit(false) {}
    };

#endif