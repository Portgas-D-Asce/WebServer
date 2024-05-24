#include <iostream>
#include "tcp/tcp_server.h"
#include "multiplex/select.h"
#include "multiplex/poll.h"
//#include "multiplex/epoll.h"


int main(int argc, char** argv) {
    if(argc < 2) exit(1);
    TcpServer<Select, Select> server(atoi(argv[1]));
    server.start();
    return 0;
}
