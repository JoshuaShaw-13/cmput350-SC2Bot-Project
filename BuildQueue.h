/**
 * Specialized Queue encapsulation for tracking the build order
 */
#ifndef BUILDQUEUE_H
#define BUILDQUEUE_H

#include <deque>
#include <sc2api/sc2_typeenums.h>



struct BuildOrderItem{
    /** struct for holding BuildOrderItem information */
    int supply;                   // Supply count at which to build the gien BuildOrderItem/ability (0 if ASAP)
    sc2::UNIT_TYPEID unit_type;   // BuildOrderItem or structure to build
    sc2::ABILITY_ID ability;      // Ability to use (for upgrades or morphs)
    bool is_unit;                 // True if BuildOrderItem_type is valid, false if ability is valid

    // Constructor for BuildOrderItems and structures
    BuildOrderItem(int s, sc2::UNIT_TYPEID u)
        : supply(s), unit_type(u), ability(sc2::ABILITY_ID::INVALID), is_unit(true) {}

    // Constructor for abilities
    BuildOrderItem(int s, sc2::ABILITY_ID a)
        : supply(s), unit_type(sc2::UNIT_TYPEID::INVALID), ability(a), is_unit(false) {}
    };

class BuildQueue{
    public:
        BuildQueue();//constructor
        ~BuildQueue();//deconstructor
        BuildQueue(const BuildQueue &q);//copy constructor
        BuildQueue &operator=(const BuildQueue &q);//assignment operator

        //getter(s)
        std::deque<BuildOrderItem> getQueue() const;

        //methods
        bool isEmpty() const;/** returns true if the queue is empty */
        BuildOrderItem peek() const;/** returns a pointer to the next building in the queue */
        BuildOrderItem pop(); /** removes the next building in the queue, and returns a pointer to that building */
        void push(const BuildOrderItem &BuildOrderItem);/**  adds the given BuildOrderItem to the back of the queue*/
        void push_front(const BuildOrderItem& item); /** Adds the given BuildOrderItem to the front of the queue */

    private:
    //data
    std::deque<BuildOrderItem> data;

};



#endif