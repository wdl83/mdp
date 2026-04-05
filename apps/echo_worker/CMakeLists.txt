project(mdp_echo_worker CXX)

add_executable(
    ${PROJECT_NAME}
        src/worker.cpp
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        mdp_worker_lib
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
