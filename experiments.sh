#!/bin/bash

# go to path holding BasicSc2Bot.exe
cd build/bin/

# Things to parameterize
#  - Enemy Race (Terran, Zerg, Protoss)
#  - Enemy Difficulty (Beginner, Easy, Moderate, Hard, Insane)
#  - Map (CactusValleyLE.SC2Map, BelShirVestigeLE.SC2Map, and ProximaStationLE.SC2Map)
#  - Number of Roaches in army (???)
#  - Number of Zerglings in army (???)

# execute BasicSc2Bot.exe
for race in zerg terran protoss; do
    for diff in Beginner Easy Moderate Hard Insane; do
        for map in CactusValleyLE.SC2Map BelShirVestigeLE.SC2Map ProximaStationLE.SC2Map; do
            ./BasicSc2Bot.exe -c -a $race -d $diff -m $map
        done
    done
done

# Collect and log results (our win or enemy win)