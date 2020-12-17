project( ${LFGE_PREFIX}_client LANGUAGES CXX DESCRIPTION "Simple Client for the Lock free version of grid engine" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable( ${LFGE_PREFIX}_client
    entry.cpp)

target_link_libraries( ${PROJECT_NAME} ${LFGE_PREFIX}_core )
target_link_libraries( ${PROJECT_NAME} caf_core )
target_link_libraries( ${PROJECT_NAME} caf_io )

target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/include) 