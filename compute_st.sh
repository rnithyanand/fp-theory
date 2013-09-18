#!/bin/bash

rm *.txt
rm compute_ST
rm core

./clear_out.sh

g++ -g -std=c++11 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_ST.cpp -lm -o compute_ST

num=$(awk 'BEGIN{for(i=1;i<=5;i+=0.25)print i}')


#for i in {80..100..10}
#do
	for j in $num
	do
		for ((k = $1 ; k <= $2 ; k ++));
		do
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 80, Threshold: 80"
			./compute_ST $k 80 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 90, Threshold: 80"
			./compute_ST $k 90 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 100, Threshold: 80"
			./compute_ST $k 100 80 $j &
			k=$((k+1))
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 80, Threshold: 80"
			./compute_ST $k 80 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 90, Threshold: 80"
			./compute_ST $k 90 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 100, Threshold: 80"
			./compute_ST $k 100 80 $j &
			k=$((k+1))
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 80, Threshold: 80"
			./compute_ST $k 80 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 90, Threshold: 80"
			./compute_ST $k 90 80 $j &
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: 100, Threshold: 80"
			./compute_ST $k 100 80 $j
			done	
	done	
#done
