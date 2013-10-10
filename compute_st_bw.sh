#!/bin/bash
rm *.txt
rm compute_ST
rm core
./clear_out.sh
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_ST.cpp -lm -o compute_ST
for ((k = $1 ; k <= $2 ; k ++));
do

        ./compute_ST $k 100 80 2.5 &
	k=$((k+1))
        ./compute_ST $k 100 80 2.5 &
        k=$((k+1))
        ./compute_ST $k 100 80 2.5 &
        k=$((k+1))
        ./compute_ST $k 100 80 2.5

        ./compute_ST $k 100 80 1.5 &
	k=$((k+1))
        ./compute_ST $k 100 80 1.5 &
        k=$((k+1))
        ./compute_ST $k 100 80 1.5 &
        k=$((k+1))
        ./compute_ST $k 100 80 1.5

done
