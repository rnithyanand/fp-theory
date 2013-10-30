#!/bin/bash
rm *.txt
rm compute_ST
rm core
./clear_out.sh
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_ST.cpp -lm -o compute_ST
for ((k = $1 ; k <= $2 ; k ++));
do
        ./compute_ST $k 5 5 .5 1 &
        ./compute_ST $k 5 5 .65 1 &
        ./compute_ST $k 5 5 .8 1 &
	./compute_ST $k 5 5 .95 1

        ./compute_ST $k 5 5 .5 2 &
        ./compute_ST $k 5 5 .65 2 &
        ./compute_ST $k 5 5 .8 2 &
	./compute_ST $k 5 5 .95 2
        
	./compute_ST $k 5 5 .5 3 &
        ./compute_ST $k 5 5 .65 3 &
        ./compute_ST $k 5 5 .8 3 &
	./compute_ST $k 5 5 .95 3

        k=$((k+1))

        ./compute_ST $k 5 5 .5 1 &
        ./compute_ST $k 5 5 .65 1 &
        ./compute_ST $k 5 5 .8 1 &
	./compute_ST $k 5 5 .95 1

        ./compute_ST $k 5 5 .5 2 &
        ./compute_ST $k 5 5 .65 2 &
        ./compute_ST $k 5 5 .8 2 &
	./compute_ST $k 5 5 .95 2
        
	./compute_ST $k 5 5 .5 3 &
        ./compute_ST $k 5 5 .65 3 &
        ./compute_ST $k 5 5 .8 3 &
	./compute_ST $k 5 5 .95 3
done
