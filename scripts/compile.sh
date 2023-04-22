#!/bin/bash

for i in `seq 4 $1`
do
    gcc ./$i-points/main.c -o ./$i-points/main.exe
done
echo "Programs compiled"
