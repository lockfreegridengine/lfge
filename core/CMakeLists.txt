project( ${LFGE_PREFIX}_core LANGUAGES CXX DESCRIPTION "Core Lock free version of grid engine" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable( ${PROJECT_NAME} entry.cpp )

target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/cube/include) 
target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/cube/modules)
message(${LFGE_BINARY_DIRECTORY})
target_link_libraries( ${PROJECT_NAME} qb-core )