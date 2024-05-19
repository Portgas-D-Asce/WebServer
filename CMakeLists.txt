cmake_minimum_required(VERSION 3.26)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp
        tcp_server.cpp
        tcp_server.h
        select.cpp
        select.h
        socket.cpp
        socket.h
        tcp_connection.cpp
        tcp_connection.h
        thread_pool_copy.cpp
        thread_pool_copy.h
        handler.cpp
        handler.h
        main_reactor.cpp
        main_reactor.h
        sub_reactor.cpp
        sub_reactor.h
)

add_executable(client client.cpp
        tcp_client.cpp
        tcp_client.h
        socket.cpp
        socket.h
)
