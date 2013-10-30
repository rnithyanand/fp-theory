#!/bin/bash
rm *.txt
rm compute_HT
rm core
./clear_out.sh
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_HT.cpp -lm -o compute_HT
for ((k = $1 ; k <= $2 ; k ++));
do
       ./compute_HT $k 98 1 .8  &
	./compute_HT $k 98 1 .95 &
	./compute_HT $k 98 1 1 &

        ./compute_HT $k 98 2 .8  &
	./compute_HT $k 98 2 .95 &
	./compute_HT $k 98 2 1  &
        
       ./compute_HT $k 98 3 .8 &
	./compute_HT $k 98 3 .95 &
	./compute_HT $k 98 3 1
done
