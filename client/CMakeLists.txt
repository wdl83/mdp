project(mdp_client_lib CXX)

add_library(
    ${PROJECT_NAME} STATIC
    src/Client.cpp
    src/ZMQClientContext.cpp
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
