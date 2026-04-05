project(mdp_broker_lib CXX)

add_library(
    ${PROJECT_NAME} STATIC
    src/Broker.cpp
    src/ZMQBrokerContext.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        include
)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        ensure
        mdp_common_lib
        zmqpp
)

install(DIRECTORY include/mdp DESTINATION include)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
