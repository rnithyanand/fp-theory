#!/bin/bash
rm *.txt
rm compute_HT
rm core
./clear_out.sh
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_HT.cpp -lm -o compute_HT
for ((k = $1 ; k <= $2 ; k ++));
do
        ./compute_HT $k 68 1 .5 &
        ./compute_HT $k 68 1 .65  &
        ./compute_HT $k 68 1 .8  &
	./compute_HT $k 68 1 .95 &
	./compute_HT $k 68 1 1 

        ./compute_HT $k 68 2 .5  &
        ./compute_HT $k 68 2 .65 &
        ./compute_HT $k 68 2 .8  &
	./compute_HT $k 68 2 .95 &
	./compute_HT $k 68 2 1  
        
	./compute_HT $k 68 3 .5 &
        ./compute_HT $k 68 3 .65 &
        ./compute_HT $k 68 3 .8 &
	./compute_HT $k 68 3 .95 &
	./compute_HT $k 68 3 1
done
