#include <iostream>
#include "tcp/tcp_server.h"
#include "multiplex/select.h"
#include "multiplex/poll.h"
#include "multiplex/epoll.h"


int main() {
    TcpServer<Select, Select> server(7000);
    server.start();
    return 0;
}
