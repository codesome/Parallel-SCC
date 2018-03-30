set_source_files_properties(scripts/PTarjan.cpp PROPERTIES COMPILE_FLAGS -O3)
cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_STANDARD 14)

project (SCC)

add_executable(MAIN_THREAD_DYNAMIC_NQUEUE
	src/SCC_MAIN_THREAD_DYNAMIC_NQUEUE.cpp
)

add_executable(MAIN_THREAD_STATIC_NQUEUE
	src/SCC_MAIN_THREAD_STATIC_NQUEUE.cpp
)

add_executable(STATIC_NXT
	src/SCC_STATIC_NXT.cpp
)

add_executable(TEST
	src/TEST.cpp
)

add_executable(PTARJAN
	src/PTarjan.cpp
)

set_source_files_properties(scripts/Tarjan2.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(TARJAN
	src/Tarjan2.cpp
)

set_source_files_properties(scripts/PTarjan.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(RPTARJAN
	src/PTarjan.cpp
)

set_source_files_properties(scripts/GenerateGraph.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(GenerateGraph 
	scripts/GenerateGraph.cpp
)

set_source_files_properties(scripts/GG.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(GG 
	scripts/GG.cpp
)

target_link_libraries (MAIN_THREAD_DYNAMIC_NQUEUE pthread)
target_link_libraries (MAIN_THREAD_STATIC_NQUEUE pthread)
target_link_libraries (STATIC_NXT pthread)
target_link_libraries (TEST pthread)
target_link_libraries (PTARJAN pthread)
target_link_libraries (RPTARJAN pthread)

include_directories("./include")
