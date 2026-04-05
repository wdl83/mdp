project(mdp_worker_lib CXX)

add_library(
    ${PROJECT_NAME} STATIC
    src/Worker.cpp
    src/WorkerTask.cpp
    src/ZMQWorkerContext.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        include
)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        mdp_common_lib
)

install(DIRECTORY include/mdp DESTINATION include)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
