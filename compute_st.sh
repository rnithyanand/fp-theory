#!/bin/bash

rm -r 80*
rm *.txt
rm compute_ST
rm core

./clear_out.sh

g++ -g -std=c++11 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp compute_ST.cpp -lm -o compute_ST

num=$(awk 'BEGIN{for(i=1;i<=10;i+=0.1)print i}')


for i in {80..120..20}
do
	for j in $num
	do
		for k in {1..100..1}
		do
			echo -e "\n Site: $k, Time Multiplier: $j, Trials: $i, Threshold: 80"
			./compute_ST $k $i 80 $j 
		done	
	done
	
done
