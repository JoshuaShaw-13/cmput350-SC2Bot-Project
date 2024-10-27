/**
 * Specialized Queue encapsulation for tracking the build order
 */

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
        std::queue<Unit*>* getQueue() const{}

        //methods
        bool isEmpty() const{/** returns true if the queue is empty */ }
        Unit* peek() const{/** returns a pointer to the next building in the queue */}
        Unit* pop() {/** removes the next building in the queue, and returns a pointer to that building */};
        void push(const Unit &unit){/**  adds the given Unit to the queue*/}

    private:
    //data
    std::queue<Unit*>* data;

};

struct Unit{
    /** struct for holding Unit information */
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