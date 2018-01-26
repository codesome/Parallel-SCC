#!/bin/bash

mkdir -p test_cases

echo "Gen 1"
./build/GenerateGraph 5000 10000 test_cases/batch1.txt
echo "Gen 2"
./build/GenerateGraph 6000 12000 test_cases/batch2.txt
echo "Gen 3"
./build/GenerateGraph 7000 14000 test_cases/batch3.txt
echo "Gen 4"
./build/GenerateGraph 8000 16000 test_cases/batch4.txt
echo "Gen 5"
./build/GenerateGraph 9000 18000 test_cases/batch5.txt
echo "Gen 6"
./build/GenerateGraph 10000 20000 test_cases/batch6.txt
echo "Gen 7"
./build/GenerateGraph 11000 22000 test_cases/batch7.txt
echo "Gen 8"
./build/GenerateGraph 12000 24000 test_cases/batch8.txt
echo "Gen 9"
./build/GenerateGraph 13000 26000 test_cases/batch9.txt
echo "Gen 10"
./build/GenerateGraph 14000 28000 test_cases/batch10.txt