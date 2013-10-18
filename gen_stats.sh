#!/bin/bash
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp generate_stats.cpp -o generate_stats

for j in {1..500..1}
do
./generate_stats $j
done
