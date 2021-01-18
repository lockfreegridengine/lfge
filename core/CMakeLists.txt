project( ${LFGE_PREFIX}_core LANGUAGES CXX DESCRIPTION "Core Library Lock free version of grid engine" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library( ${PROJECT_NAME} SHARED 
        src/entry.cpp 
        src/heartbeat.cpp
        src/lfge_errors.cpp
        src/logger.cpp
        )

target_link_libraries( ${PROJECT_NAME} caf_core )

target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/include) 
#target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/cube/modules)
#message(${LFGE_BINARY_DIRECTORY})

#---------- Build unit tests-------------------

if(NOT LFGE_ENABLE_TESTING)
  return()
endif()

add_executable( ${PROJECT_NAME}_test
                test/entry.cpp
                test/test_heartbeat.cpp
                test/test_logger.cpp)
target_link_libraries( ${PROJECT_NAME}_test caf_core ${PROJECT_NAME} Threads::Threads)

target_include_directories( ${PROJECT_NAME}_test PUBLIC ${LFGE_DIRECTORY}/include ${CMAKE_CURRENT_SOURCE_DIR}/test) 

if(MSVC)
  target_link_libraries(${PROJECT_NAME}_test iphlpapi)
endif()

lfge_add_test_suites(
  test_heartbeat
)