#!/bin/bash
g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp select_st.cpp -o select_st

for j in {1..1000..1}
do
#	./select_st $j 80 80  #(site, threshold = 80 90 100) trials 80
#	./select_st $j 90 80
	./select_st $j 100 80
#	./select_st $j 90 90
#	./select_st $j 100 90
#	./select_st $j 100 100
done

