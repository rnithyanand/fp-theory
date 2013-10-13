#!/bin/bash

rm *.txt
rm compute_HT
rm core

./clear_out.sh

g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_HT.cpp -lm -o compute_HT

#num = $(awk 'BEGIN{for(i=1;i<=2.5;i+=1)print i}')

for(( k=$1 ; k <= $2 ; k++ ));
do
	./compute_HT $k 64 1 1 &
	./compute_HT $k 64 2 1 &
	./compute_HT $k 64 3 1 &
	./compute_HT $k 64 4 1 &
	./compute_HT $k 64 5 1 &
	./compute_HT $k 64 6 1 

	./compute_HT $k 64 1 1.5 &
	./compute_HT $k 64 2 1.5 &
	./compute_HT $k 64 3 1.5 &
	./compute_HT $k 64 4 1.5 &
	./compute_HT $k 64 5 1.5 &
	./compute_HT $k 64 6 1.5 
	
	./compute_HT $k 64 1 2 &
	./compute_HT $k 64 2 2 &
	./compute_HT $k 64 3 2 &
	./compute_HT $k 64 4 2 &
	./compute_HT $k 64 5 2 &
	./compute_HT $k 64 6 2 
	

done
