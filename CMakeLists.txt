cmake_minimum_required(VERSION 3.10)
project(mdp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    -Wall
    -Werror
    -Wextra
    -Wfatal-errors
    -Wno-parentheses
    -Wno-unused-function
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wsequence-point
    -Wshadow
    -Wswitch
    -Wunreachable-code
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -fsanitize=address)
    add_link_options(-fsanitize=address)
else()
    add_compile_options(-O2 -flto)
endif()

add_definitions(-DENABLE_TRACE)

include (CTest)
find_package(GTest REQUIRED)

add_library(ensure INTERFACE)
target_include_directories(ensure INTERFACE "${CMAKE_SOURCE_DIR}/modules")

add_subdirectory(common)
add_subdirectory(broker)
add_subdirectory(worker)
add_subdirectory(client)
add_subdirectory(apps)
add_subdirectory(modules/zmqpp)
add_subdirectory(tests)

if(BUILD_TESTING)
add_custom_target(
    run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endif(BUILD_TESTING)
