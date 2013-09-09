#!/bin/bash

rm *.txt
rm test_fileops
rm core

./clear_out.sh

g++ -g -std=c++11 packets.cpp traces.cpp file_ops.cpp test_fileops.cpp -lm -o test_fileops

for i in {1..100..1}
do 
	for j in {1..100..1}
	do
		echo "Reading trace $i, $j"
		./test_fileops $i $j
	done
done
