#!/bin/bash

mkdir -p test_cases

for i in {1..6}
do
	echo "Sparse ${i}e6: ./build/GG ${i}000000 $(( i * 7 )) 0.5 test_cases/sparse_${i}e6.txt" &
	./build/GG ${i}000000 $(( i * 7 )) 0.5 test_cases/sparse_${i}e6.txt &
done

for i in {1..6}
do
	echo "Dense ${i}e6: ./build/GG ${i}000000 $(( i * 100 )) 0.85 test_cases/dense_${i}e6.txt" &
	./build/GG ${i}000000 $(( i * 100 )) 0.85 test_cases/dense_${i}e6.txt &
done

wait
