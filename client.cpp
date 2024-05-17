#include <iostream>
#include <vector>

#include "tcp_client.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    TcpClient client{};
    client.connect("127.0.0.1", 30000);
    client.run();

    //int n = 1000;
    //std::vector<TcpClient> clients(n);
    //for(int i = 0; i < n; ++i) {
    //    clients[i].connect("127.0.0.1", 30000);
    //}
    //while(1){};
    return 0;
}
