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
./BasicSc2Bot.exe -c -a zerg -d Hard -m CactusValleyLE.SC2Map

# Collect and log results (our win or enemy win)