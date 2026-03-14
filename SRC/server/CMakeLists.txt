set(
    SERVER_SRC
    connect.cpp
    sermysql.cpp
    server.cpp
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
add_executable(server ${SERVER_SRC})
target_link_libraries(server event)
target_link_libraries(server ${JSONCPP_LIBRARIES})
target_include_directories(server PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(server  mysqlclient)
target_link_libraries(server pthread)