project( ${LFGE_PREFIX}_core LANGUAGES CXX DESCRIPTION "Core Library Lock free version of grid engine" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library( ${PROJECT_NAME} SHARED entry.cpp Heartbeat.cpp)

target_link_libraries( ${PROJECT_NAME} caf_core )

target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/include/core) 
#target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/cube/modules)
#message(${LFGE_BINARY_DIRECTORY})