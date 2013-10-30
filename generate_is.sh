#!/bin/bash

g++ -g -std=c++11 -O3 packets.cpp traces.cpp file_ops.cpp ipstats.cpp -o ipstats

for i in {1..500..1}
do
		./ipstats $i
done

