#!/bin/bash

g++ src/Tarjan2.cpp --std=c++14 -O3 -o build/t3 -pthread
g++ src/PTarjan.cpp --std=c++14 -O3 -o build/p3 -pthread
g++ src/SCC_STATIC_NXT.cpp --std=c++14 -O3 -o build/or3 -pthread -I ./include