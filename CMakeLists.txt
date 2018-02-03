cmake_minimum_required (VERSION 2.6)

project (SCC)

add_executable(SCC 
	src/SCC.cpp
)

add_executable(WFSCCQUEUE
	src/SCC_WFSCCQUEUE.cpp
)

add_executable(WFPOOL
	src/SCC_WFPOOL.cpp
)

add_executable(SEM
	src/SCC_SEM.cpp
)

add_executable(SEM_WFPOOL
	src/SCC_SEM_WFPOOL.cpp
)

add_executable(REMOVED_PHASE3
	src/SCC_REMOVED_PHASE3.cpp
)

add_executable(STATIC_ASSIGNMENT
	src/SCC_STATIC_ASSIGNMENT.cpp
)

add_executable(MAIN_THREAD_STATIC
	src/SCC_MAIN_THREAD_STATIC.cpp
)

add_executable(MAIN_THREAD_DYNAMIC
	src/SCC_MAIN_THREAD_DYNAMIC.cpp
)

add_executable(GenerateGraph 
	scripts/GenerateGraph.cpp
)

target_link_libraries (SCC pthread)
target_link_libraries (WFSCCQUEUE pthread)
target_link_libraries (WFPOOL pthread)
target_link_libraries (SEM pthread)
target_link_libraries (SEM_WFPOOL pthread)
target_link_libraries (REMOVED_PHASE3 pthread)
target_link_libraries (STATIC_ASSIGNMENT pthread)
target_link_libraries (MAIN_THREAD_STATIC pthread)
target_link_libraries (MAIN_THREAD_DYNAMIC pthread)

include_directories("./include")
