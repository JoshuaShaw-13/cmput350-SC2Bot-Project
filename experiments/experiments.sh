#!/bin/bash

# go to path holding BasicSc2Bot.exe
cd ../build/bin/

# Things to test
#  - Enemy Race (terran, zerg, protoss)
#  - Enemy Difficulty (Beginner, Easy, Moderate, Hard, Insane)
#  - Map (CactusValleyLE.SC2Map, BelShirVestigeLE.SC2Map, ProximaStationLE.SC2Map)
#  - Number of Roaches in group (5-10)
#  - Number of additional drones built after build order completion (4-12)

# clear bot_results.txt
echo "" > ../../experiments/bot_results.txt

# loop through all testing parameters
for race in zerg terran protoss; do # enemy race
    for diff in Beginner Easy Moderate Hard Insane; do # enemy difficulty
        for map in CactusValleyLE.SC2Map BelShirVestigeLE.SC2Map ProximaStationLE.SC2Map; do # map used
            for r_amount in 5 6 7 8 9 10; do # amount of roaches in group
                for dr_amount in 4 6 8 10 12; do # amount of additional drones
                    # set counts to 0 for this parameter set
                    win_count=0
                    loss_count=0
                    tie_count=0
                    timeout_count=0
                    echo "$race, $diff, $map, $r_amount, $dr_amount: " >> ../../experiments/bot_results.txt # write parameter set into results file
                    for run in 1 2 3 4 5; do # goes through five runs of this parameter set
                        ./BasicSc2Bot.exe -c -a $race -d $diff -m $map -r $r_amount -dr $dr_amount > ../../experiments/match_output.txt # runs bot with parameter set
                        if grep -q "Timeout" ../../experiments/match_output.txt; then # counts timeouts
                            let timeout_count+=1
                        elif grep -q "Victory" ../../experiments/match_output.txt; then # counts victories
                            let win_count+=1
                        elif grep -q "Defeat" ../../experiments/match_output.txt; then # counts defeaats
                            let loss_count+=1
                        elif grep -q "Tie" ../../experiments/match_output.txt; then # counts ties
                            let tie_count+=1
                        fi
                    done
                    # writes results for this parameter set into results file for consideration
                    echo "Wins: $win_count" >> ../../experiments/bot_results.txt
                    echo "Losses: $loss_count" >> ../../experiments/bot_results.txt
                    echo "Ties: $tie_count" >> ../../experiments/bot_results.txt
                    echo "Timeouts: $timeout_count" >> ../../experiments/bot_results.txt
                    echo "" >> ../../experiments/bot_results.txt
                done
            done
        done
    done
done
