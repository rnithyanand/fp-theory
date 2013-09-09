#!/bin/bash

rm *.txt
rm test_STops
rm core

./clear_out.sh

g++ -g -std=c++11 packets.cpp traces.cpp file_ops.cpp supertrace_helpers.cpp supertrace_func.cpp test_STops.cpp -lm -o test_STops

for i in {1..100..1}
do
#	for j in {90..100..1}
#	do
		echo -e "\nSite: $i, Trials: 100, Thresh: 80"
		./test_STops $i 100 80
		echo -e "\n"
#	done
done

