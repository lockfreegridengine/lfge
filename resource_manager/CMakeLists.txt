project( ${LFGE_PREFIX}_resource_manager LANGUAGES CXX DESCRIPTION "Resource Manager for the Lock free version of grid engine" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library( ${LFGE_PREFIX}_resource_manager 
    src/registering_service.cpp
    src/service_manager.cpp)

target_link_libraries( ${PROJECT_NAME} ${LFGE_PREFIX}_core )
target_link_libraries( ${PROJECT_NAME} caf_core )
target_link_libraries( ${PROJECT_NAME} caf_io )

target_include_directories( ${PROJECT_NAME} PUBLIC ${LFGE_DIRECTORY}/include) 

#---------- Build unit tests-------------------

if(NOT LFGE_ENABLE_TESTING)
  return()
endif()

add_executable( ${PROJECT_NAME}_test
                test/entry.cpp
                test/test_service_manager.cpp)
target_link_libraries( ${PROJECT_NAME}_test caf_core ${PROJECT_NAME} Threads::Threads)

target_include_directories( ${PROJECT_NAME}_test PUBLIC ${LFGE_DIRECTORY}/include ${CMAKE_CURRENT_SOURCE_DIR}/test) 

if(MSVC)
  target_link_libraries(${PROJECT_NAME}_test iphlpapi)
endif()

lfge_add_test_suites(
  test_heartbeat
)