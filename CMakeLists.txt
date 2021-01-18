#Minimum required CMake
cmake_minimum_required(VERSION 3.10)

#Setting project details
project( lfge LANGUAGES CXX DESCRIPTION "A lock free grid engine" )
set(LFGE_PREFIX ${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Setting versions
set(LFGE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(LFGE_VERSION_MAJOR 1)
set(LFGE_VERSION_MINOR 0)
set(LFGE_VERSION_PATCH 0)
set(LFGE_VERSION_TWEAK 0)
set(LFGE_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

option(LFGE_ENABLE_TESTING "Build unit test suites" ON)

find_package(CAF)


if(LFGE_ENABLE_TESTING)
    enable_testing()
    function(lfge_add_test_suites target)

    foreach(suiteName ${ARGN})
        string(REPLACE "." "/" suitePath ${suiteName})
        target_sources(${target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/test/${suitePath}.cpp")
      add_test(NAME ${suiteName}
               COMMAND ${target} -r300 -n -v5 -s"^${suiteName}$")
    endforeach()
    endfunction()
endif()


add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(resource_manager)
add_subdirectory(auto_allocator)
#add_subdirectory(thirdparty/cube)
#add_subdirectory(thirdparty/zmq)
add_subdirectory(thirdparty/caf)
