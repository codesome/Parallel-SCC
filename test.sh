#!/bin/bash

for i in {1..5}
do
	echo "Test s${i} 1e5"
	./build/MAIN_THREAD_STATIC_NQUEUE test_cases/1e5_1e5.txt $1 > test_cases/static_test${i}_1e5.txt
	echo "Test s${i} 1.5e5"
	./build/MAIN_THREAD_STATIC_NQUEUE test_cases/1e5_1.5e5.txt $1 > test_cases/static_test${i}_1.5e5.txt
	echo "Test s${i} 2e5"
	./build/MAIN_THREAD_STATIC_NQUEUE test_cases/1e5_2e5.txt $1 > test_cases/static_test${i}_2e5.txt
	echo "Test s${i} 2.5e5"
	./build/MAIN_THREAD_STATIC_NQUEUE test_cases/1e5_2.5e5.txt $1 > test_cases/static_test${i}_2.5e5.txt
	echo "Test s${i} 3e5"
	./build/MAIN_THREAD_STATIC_NQUEUE test_cases/1e5_3e5.txt $1 > test_cases/static_test${i}_3e5.txt

	echo "Test d${i} 1e5"
	./build/MAIN_THREAD_DYNAMIC_NQUEUE test_cases/1e5_1e5.txt $1 > test_cases/dynamic_test${i}_1e5.txt
	echo "Test d${i} 1.5e5"
	./build/MAIN_THREAD_DYNAMIC_NQUEUE test_cases/1e5_1.5e5.txt $1 > test_cases/dynamic_test${i}_1.5e5.txt
	echo "Test d${i} 2e5"
	./build/MAIN_THREAD_DYNAMIC_NQUEUE test_cases/1e5_2e5.txt $1 > test_cases/dynamic_test${i}_2e5.txt
	echo "Test d${i} 2.5e5"
	./build/MAIN_THREAD_DYNAMIC_NQUEUE test_cases/1e5_2.5e5.txt $1 > test_cases/dynamic_test${i}_2.5e5.txt
	echo "Test d${i} 3e5"
	./build/MAIN_THREAD_DYNAMIC_NQUEUE test_cases/1e5_3e5.txt $1 > test_cases/dynamic_test${i}_3e5.txt
done