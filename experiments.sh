#!/bin/bash

# go to path holding BasicSc2Bot.exe
cd build/bin/

# Things to parameterize
#  - Enemy Race (Terran, Zerg, Protoss)
#  - Enemy Difficulty (Beginner, Easy, Moderate, Hard, Insane)
#  - Map (CactusValleyLE.SC2Map, BelShirVestigeLE.SC2Map, and ProximaStationLE.SC2Map)
#  - Number of Roaches in army (3 - 12)

# execute BasicSc2Bot.exe
for race in zerg terran protoss; do
    for diff in Easy Moderate Hard; do #additional options Beginner, Insane
        for map in CactusValleyLE.SC2Map BelShirVestigeLE.SC2Map ProximaStationLE.SC2Map; do
            for r_amount in 3 6 9 12; do
                ./BasicSc2Bot.exe -c -a $race -d $diff -m $map
            done
        done
    done
done

#./BasicSc2Bot.exe -c -a zerg -d Hard -m CactusValleyLE.SC2Map > match_output.txt

# Collect and log results (our win or enemy win)