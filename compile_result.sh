#!/bin/bash

cat test_cases/static_test*_1e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/static_1e5.txt
cat test_cases/static_test*_1.5e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/static_1.5e5.txt
cat test_cases/static_test*_2e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/static_2e5.txt
cat test_cases/static_test*_2.5e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/static_2.5e5.txt
cat test_cases/static_test*_3e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/static_3e5.txt

cat test_cases/dynamic_test*_1e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/dynamic_1e5.txt
cat test_cases/dynamic_test*_1.5e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/dynamic_1.5e5.txt
cat test_cases/dynamic_test*_2e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/dynamic_2e5.txt
cat test_cases/dynamic_test*_2.5e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/dynamic_2.5e5.txt
cat test_cases/dynamic_test*_3e5.txt | grep "Time:" | sed 's/.*Time: \(.*\)/\1/g' >> test_cases/dynamic_3e5.txt
