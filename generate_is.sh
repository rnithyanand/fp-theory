#!/bin/bash

g++ -g -std=c++11 -O3 packets.cpp traces.cpp file_ops.cpp generate_inputstats.cpp -o gen_is

for i in {1..100..1}
do
	for j in {1..100..1}
	do
		./gen_is $i $j
	done
done

