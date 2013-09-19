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

#g++ single_param_clustering.cpp  compute_lb.cpp -o compute_lb
g++ -O3 clustering_single_param.cpp -o cluster 

echo "Computing optimal allocation overhead"

for i in `seq 1 $((1+n/2))`
do
	echo "Computing non-uniform security overhead"
	./cluster $n $i 0 $sizestring
	echo "Computing uniform security overhead"
	./cluster $n $i 1 $sizestring

      #  ./compute_lb $n $i $sizestring
done

