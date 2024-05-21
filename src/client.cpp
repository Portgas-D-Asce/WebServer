#include <iostream>
#include <vector>
#include "tcp/tcp_client.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    TcpClient client{};
    client.connect("127.0.0.1", 30000);
    client.run1();

    //int n = 100;
    //std::vector<TcpClient> clients(n);
    //for(int i = 0; i < n; ++i) {
    //    clients[i].connect("127.0.0.1", 30000);
    //    clients[i].auto_run();
    //}
    //while(1){};
    return 0;
}
