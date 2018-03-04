cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_STANDARD 14)

project (SCC)

add_executable(SCC 
	src/SCC.cpp
)

add_executable(MAIN_THREAD_STATIC
	src/SCC_MAIN_THREAD_STATIC.cpp
)

add_executable(MAIN_THREAD_DYNAMIC
	src/SCC_MAIN_THREAD_DYNAMIC.cpp
)

add_executable(MAIN_THREAD_DYNAMIC_NQUEUE
	src/SCC_MAIN_THREAD_DYNAMIC_NQUEUE.cpp
)

add_executable(MAIN_THREAD_STATIC_NQUEUE
	src/SCC_MAIN_THREAD_STATIC_NQUEUE.cpp
)

set_source_files_properties(scripts/GenerateGraph.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(GenerateGraph 
	scripts/GenerateGraph.cpp
)

set_source_files_properties(scripts/GG.cpp PROPERTIES COMPILE_FLAGS -O3)
add_executable(GG 
	scripts/GG.cpp
)

target_link_libraries (SCC pthread)
target_link_libraries (MAIN_THREAD_STATIC pthread)
target_link_libraries (MAIN_THREAD_DYNAMIC pthread)
target_link_libraries (MAIN_THREAD_DYNAMIC_NQUEUE pthread)
target_link_libraries (MAIN_THREAD_STATIC_NQUEUE pthread)

include_directories("./include")
