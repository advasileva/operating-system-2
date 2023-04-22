#!/bin/bash

for i in `seq 4 $1`
do
    for j in `seq 1 $2`
    do
        rm -f $i-points/test$j.out
    done
done
echo "Tests output deleted"
