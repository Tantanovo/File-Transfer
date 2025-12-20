set(
    CLIENT_SRC
    connect.cpp
    client.cpp
)
find_package(jsoncpp REQUIRED)
add_executable(client ${CLIENT_SRC})
target_link_libraries(client event)
target_link_libraries(client JsonCpp::JsonCpp)
