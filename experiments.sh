#!/bin/bash

# go to path holding BasicSc2Bot.exe
cd build/bin/

# Things to test
#  - Enemy Race (terran, zerg, protoss)
#  - Enemy Difficulty (Beginner, Easy, Moderate, Hard, Insane)
#  - Map (CactusValleyLE.SC2Map, BelShirVestigeLE.SC2Map, ProximaStationLE.SC2Map)
#  - Number of Roaches in group (5-10)
#  - Number of additional Roaches after build order completion (4-12)
    
# execute BasicSc2Bot.exe
for race in zerg terran protoss; do
    for diff in Beginner Easy Moderate Hard Insane; do
        for map in CactusValleyLE.SC2Map BelShirVestigeLE.SC2Map ProximaStationLE.SC2Map; do
            for r_amount in 5 6 7 8 9 10; do
                for dr_amount in 4 6 8 10 12; do
                    win_count=0
                    loss_count=0
                    tie_count=0
                    echo "$race, $diff, $map, $r_amount, $dr_amount: " >> ../../bot_results.txt
                    for run in 1 2 3 4 5; do
                        ./BasicSc2Bot.exe -c -a $race -d $diff -m $map -r $r_amount -dr $dr_amount > ../../match_output.txt
                        if grep -q "Victory" ../../match_output.txt; then
                            let win_count+=1
                        elif grep -q "Defeat" ../../match_output.txt; then
                            let loss_count+=1
                        elif grep -q "Tie" ../../match_output.txt; then
                            let tie_count+=1
                        fi
                    done
                    echo "Wins: $win_count" >> ../../bot_results.txt
                    echo "Losses: $loss_count" >> ../../bot_results.txt
                    echo "Ties: $tie_count" >> ../../bot_results.txt
                    echo "" >> ../../bot_results.txt
                done
            done
        done
    done
done
