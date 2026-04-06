cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(test_mdp_common)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ZMQIdentity_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MutualHeartbeatMonitor_tests.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        gtest
        gtest_main
        mdp_common_lib
)

gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)
#-------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME} DESTINATION bin)
