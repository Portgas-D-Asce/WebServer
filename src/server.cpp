#include <iostream>
#include "tcp/tcp_server.h"
#include "multiplex/select.h"
#include "multiplex/poll.h"

int main() {
    TcpServer<Select, Poll> server(30000);
    server.start();
    return 0;
}