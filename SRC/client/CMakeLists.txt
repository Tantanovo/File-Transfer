set(
    CLIENT_SRC
    connect.cpp
    client.cpp
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
add_executable(client ${CLIENT_SRC})
target_link_libraries(client event)
target_link_libraries(client ${JSONCPP_LIBRARIES})
target_include_directories(client PUBLIC ${JSONCPP_INCLUDE_DIRS})
