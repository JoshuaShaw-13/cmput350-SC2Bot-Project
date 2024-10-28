#include "BasicSc2Bot.h"
#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_unit_filters.h>
using namespace sc2;

void BasicSc2Bot::OnGameStart() { return; }

void BasicSc2Bot::OnStep() { 
    const int currentSupply = Observation() ->GetFoodUsed();
    const ObservationInterface* observation = Observation();
    // Change it to build at cap for example build drones till 13

    // build next structure/unit
    if(!buildQueue.empty()){
        auto& buildItem = buildQueue.peek();
        bool is_successful;
        int count;
        for(count = 0; count < buildItem.num; count++){
            is_successful = tryBuild(buildItem);
            if (!is_successful){
                break;
            }
            count++;
        }
        buildItem.num - count;
    }
    
    // build overseers when we are nearing troop capacity
    //buildOverseers();

    //check if we have enough resources to expand to a new base
    // this would be in the manager
    

    // Attack once we have an optimal army build
    // use the attack base queue
    if(isArmyReady()){
        if(!enemy_bases.empty()){
            launchAttack(enemy_bases[0]);
        }
    }

    return;
}
const Unit* findIdleLarva(const ObservationInterface* observation){
    for(const auto& unit: observation->GetUnits(Unit::Alliance::Self)){
        if(unit->unit_type == UNIT_TYPEID::ZERG_LARVA && unit->orders.empty()){
            return unit;
        }
    }
}
const Unit* findIdleDrone(const ObservationInterface* observation){
    for(const auto& unit: observation->GetUnits(Unit::Alliance::Self)){
        if(unit->unit_type == UNIT_TYPEID::ZERG_DRONE && unit->orders.empty()){
            return unit;
        }
    }
}
// Eventually will use manager
// For now it checks whether its a unit, strucure, or upgrade
bool tryBuild(const ObservationInterface* observation, struct buildOrderItem buildItem){
    // if its a unit build it if we can afford
    if(buildItem.is_unit){
        // check whether its a unit or a structure
        const Unit* larva = nullptr;
        switch(buildItem.unit_type){
            case UNIT_TYPEID::ZERG_DRONE:
                larva = findIdleLarva(observation);
                if(larva){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_DRONE);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_ROACH:
                larva = findIdleLarva(observation);
                if(larva && observation->GetMinerals() >= 75 && observation->GetVespene() >= 25){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ROACH);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_ZERGLING:
                larva = findIdleLarva(observation);
                if(larva && observation->GetMinerals() >= 50){
                    Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_ZERGLING);
                    return true;
                }
                break;
            
            case UNIT_TYPEID::ZERG_QUEEN:
                for(const auto& hatchery: observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))){
                    if(hatchery->orders.empty() && observation->GetMinerals() >= 150){
                        Actions()->UnitCommand(larva, ABILITY_ID::TRAIN_QUEEN);
                        return true;
                    }
                }
                break;
            
            default:
                const Unit* drone = findIdleDrone(observation);
                if(drone){
                    float rx = GetRandomScalar();
                    float ry = GetRandomScalar();
                    Point2D build_position = Point2D(drone->pos.x + rx * 15.0f, drone->pos.y + ry * 15.0f);

                    Actions()->UnitCommand(drone, buildItem.ability, build_position);
                    return true;
                }
        }
        return false;
    }
    // must be an upgrade
    if(buildItem.ability != ABILITY_ID::INVALID){
        Units structures = observation->GetUnits(Unit::Alliance::Self, IsUnits(buildItem.unit_type));
        if(!structures.empty()){
            Actions()->UnitCommand(structures[0], buildItem.ability);
            return true;
        }
    }
    return false;
}


bool isArmyReady(const ObservationInterface* observation) {
    int roach_count;
    int zergling_count;
    const int optRoach = 10;
    const int optZergling = 20;
    for (const auto& unit : observation->GetUnits(Unit::Alliance::Self)) {
        if (unit->unit_type == UNIT_TYPEID::ZERG_ROACH) {
            roach_count++;
        } else if (unit->unit_type == UNIT_TYPEID::ZERG_ZERGLING) {
            zergling_count++;
        }
    }
    return roach_count >= optRoach && zergling_count >= optZergling;
}
void launchAttack(const ObservationInterface* observation, const Point2D& target){
    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        if (IsCombatUnit(unit)) {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
        }
    }
}
// void BuildOverseersIfNeeded(const ObservationInterface* observation) {
//     if (observation->GetFoodUsed() >= observation->GetFoodCap() - 2){
//         if (builder && observation->GetMinerals() >= 100) {
//             Actions()->UnitCommand(builder, ABILITY_ID::TRAIN_OVERSEER);
//         }
//     }
// }
