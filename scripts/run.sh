#!/bin/bash

for i in `seq 4 $1`
do
    echo "Program for $i points"
    for j in `seq 1 $2`
    do
        echo "Test $j"
        ./$i-points/main.exe tests/test$j.in
    done
    echo
done
echo "Programs finished"
