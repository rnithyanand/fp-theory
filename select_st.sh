#!/bin/bash
rm log_selec*
rm selected.stats
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp select_st.cpp -o selector
for ((j = $1 ; j <= $2 ; j ++));
do
	./selector $j
done

