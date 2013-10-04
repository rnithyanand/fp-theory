#!/bin/bash

rm *.txt
rm compute_ST
rm core

./clear_out.sh

g++ -g -std=c++0x -O3 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_ST.cpp -lm -o compute_ST

#num=$(awk 'BEGIN{for(i=1;i<=5;i+=1)print i}')


#for i in {80..100..10}
#do
for ((k = $1 ; k <= $2 ; k ++));
	do
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 80, Threshold: 80"
		./compute_ST $k 80 80 1 &
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 90, Threshold: 80"
		./compute_ST $k 90 80 1 &
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 100, Threshold: 80"
		./compute_ST $k 100 80 1 &
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 80, Threshold: 80"
		./compute_ST $k 90 90 1 &
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 90, Threshold: 80"
		./compute_ST $k 100 90 1 &
		echo -e "\n Site: $k, Time Multiplier: $j, Trials: 100, Threshold: 80"
		./compute_ST $k 100 100 1  
done	


