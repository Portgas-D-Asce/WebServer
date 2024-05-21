#ifndef WEBSERVER_TCP_CLIENT_H
#define WEBSERVER_TCP_CLIENT_H
#include <string>
#include <arpa/inet.h>
#include "../common/socket.h"

class TcpClient {
private:
    Socket _sock;
public:
    void connect(const std::string& ip, int port) const {
        _sock.sock_connect(Socket::sock_address(ip, port));
        printf("connect success\n");

    }
    void run() const {
        while(true) {
            char buff[1024] = {0};
            scanf(" %[^\n]", buff);
            int cnt = 0;
            int sz = strlen(buff) + 4;
            printf("sz = %d\n", sz);
            sz = htonl(sz);
            if((cnt = _sock.sock_send((char *)&sz, sizeof(sz))) == -1) {
                perror("send type");
            }
            printf("%d\n", cnt);

            int type = 0;
            type = htonl(type);
            if((cnt = _sock.sock_send((char *)&type, sizeof(type))) == -1) {
                perror("send type");
            }
            printf("%d\n", cnt);

            if((cnt = _sock.sock_send((char *)buff, strlen(buff))) == -1) {
                perror("send");
            }
            printf("%d\n", cnt);
        }
    }

    void auto_run() const {
        int zzz = 0;
        while(true) {
            char buff[1024] = {0};
            for(int i = 0; i < 10; ++i) {
                buff[i] = 'a';
            }
            int cnt = 0;
            int sz = strlen(buff) + 4;
            printf("sz = %d\n", sz);
            sz = htonl(sz);
            if((cnt = _sock.sock_send((char *)&sz, sizeof(sz))) == -1) {
                perror("send type");
            }
            printf("%d\n", cnt);

            int type = 0;
            type = htonl(type);
            if((cnt = _sock.sock_send((char *)&type, sizeof(type))) == -1) {
                perror("send type");
            }
            printf("%d\n", cnt);

            if((cnt = _sock.sock_send((char *)buff, strlen(buff))) == -1) {
                perror("send");
            }
            printf("%d\n", cnt);
            ++zzz;
            printf("zzz = %d\n", zzz);
            //sleep(1);
        }
    }

    void run1() const {
        while(true) {
            char buff[1024] = {0};

            std::string s("GET / HTTP/1.1\r\n"
                          "Sec-Fetch-Site: none\r\n"
                          "Connection: keep-alive\r\n"
                          "Upgrade-Insecure-Requests: 1\r\n"
                          "\r\n");
            int cnt = _sock.sock_send(s.c_str(), s.size());
            printf("send: %d\n", cnt);
            cnt = recv(_sock.fd(), buff, sizeof(buff), 0);
            buff[cnt] = '\0';
            printf("%s\n", buff);
            sleep(5);

        }
    }
};
#endif //WEBSERVER_TCP_CLIENT_H
