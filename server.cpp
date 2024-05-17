#include <iostream>

#include "tcp_server.h"
#include "select.h"
#include "multiplex/poll.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    TcpServer<Select> server(30000);
    server.start();
    return 0;
}