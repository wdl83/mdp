project(mdp_common_lib CXX)

add_library(
    ${PROJECT_NAME} STATIC
    src/MutualHeartbeatMonitor.cpp
    src/ZMQIdentity.cpp
    src/utils.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        include
)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        zmqpp
        ensure
)

install(DIRECTORY include/mdp DESTINATION include)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
