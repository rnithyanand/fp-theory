#!/bin/bash
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp generate_stats.cpp -o generate_stats

for j in {1..1000..1}
do
./generate_stats $j 80 100
done
