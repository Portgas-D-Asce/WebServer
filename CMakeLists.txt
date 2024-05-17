cmake_minimum_required(VERSION 3.26)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
link_directories(${PROJECT_SOURCE_DIR}/multiplex/)
link_directories(${PROJECT_SOURCE_DIR}/dispatch/)

add_executable(server server.cpp
        tcp_server.cpp
        tcp_server.h
        select.cpp
        select.h
        socket.cpp
        socket.h
        tcp_connection.cpp
        tcp_connection.h
        thread_pool.cpp
        thread_pool.h
)

add_executable(client client.cpp
        tcp_client.cpp
        tcp_client.h
        socket.cpp
        socket.h
)
