cmake_minimum_required(VERSION 3.20)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(server src/server.cpp
        src/tcp/tcp_server.h
        src/multiplex/select.h
        src/common/socket.h
        src/common/connection.h
        src/thread/thread_pool.h
        src/reactor/main_reactor.h
        src/reactor/sub_reactor.h
        src/http/http.h
        src/handler/file_handler.h
        src/handler/directory_handler.h
        src/handler/handler.h
        src/http/http_header.h
        src/html/html.h
        src/config/config.h
        src/buffer/ring_buffer.h
)
target_link_libraries(server -lpthread)

