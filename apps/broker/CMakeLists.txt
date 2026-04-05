project(mdp_broker CXX)

add_executable(
    ${PROJECT_NAME}
    src/broker.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        mdp_broker_lib
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
