#!/bin/bash
g++ -g -std=c++11 -O3 packets.cpp traces.cpp file_ops.cpp generate_stats.cpp -o generate_stats

for j in {1..1000..1}
do
	./generate_stats $j 80 80
	./generate_stats $j 80 90
	./generate_stats $j 80 100
	./generate_stats $j 90 90
	./generate_stats $j 90 100
	./generate_stats $j 100 100
done
