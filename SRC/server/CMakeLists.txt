set(
    SERVER_SRC
    connect.cpp
    sermysql.cpp
    server.cpp
)
find_package(jsoncpp REQUIRED)
add_executable(server ${SERVER_SRC})
target_link_libraries(server event)
target_link_libraries(server JsonCpp::JsonCpp)
target_link_libraries(server  mysqlclient)
target_link_libraries(server pthread)