#!/bin/bash
n=0
echo "Reading values from input.txt"
sizestring=' '
emptystring=' '

./clear_out.sh
./compute_sizes 100 100

for line in $(cat $1)
        do sizestring=$sizestring$line$emptystring
done

for line in $(cat $1)
        do n=$((n+1))
done

g++ compute_lb.cpp -o compute_lb

echo "Computing optimal allocation overhead"

for i in `seq 1 $((1+n/2))`
do
        ./compute_lb $n $i $sizestring
done

