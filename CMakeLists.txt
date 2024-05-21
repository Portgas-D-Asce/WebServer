cmake_minimum_required(VERSION 3.26)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp
        tcp/tcp_server.h
        multiplex/select.h
        common/socket.h
        common/connection.h
        thread/thread_pool_copy.h
        reactor/main_reactor.h
        reactor/sub_reactor.h
        http/http.h
        handler/file_handler.h
        handler/directory_handler.h
        handler/handler.h
        http/http_header.h
)

add_executable(client client.cpp
        tcp/tcp_client.h
        common/socket.h
)
