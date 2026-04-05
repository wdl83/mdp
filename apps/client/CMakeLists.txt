project(mdp_client CXX)

add_executable(
    ${PROJECT_NAME}
    src/client.cpp
)
target_link_libraries(
    mdp_client
    PRIVATE
        mdp_client_lib
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
