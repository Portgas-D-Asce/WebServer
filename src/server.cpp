#include "tcp/tcp_server.h"
#include "multiplex/select.h"
#include "multiplex/poll.h"
//#include "multiplex/epoll.h"
#include "multiplex/kqueue.h"


int main(int argc, char** argv) {
    if(argc < 2) exit(1);
    TcpServer<Select, KQueue> server(atoi(argv[1]));
    server.start();
    return 0;
}
