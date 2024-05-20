//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_HTTP_RESPONSE_H
#define WEBSERVER_HTTP_RESPONSE_H
#include <string>

class HttpResponse {
private:
    std::map<std::string, std::string> _mp;
public:
    std::string not_found() {
        std::string s("HTTP/1.0 404 NOT FOUND\r\n"
                      "Server: jdbhttpd/0.1.0\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: 31\r\n"
                      "\r\n"
                      "<HTML><TITLE>Not</TITLE></HTML>\r\n"
                      );
        return s;
    }

    std::string wrap(const std::string& s) {
        return s;
    }
};


#endif //WEBSERVER_HTTP_RESPONSE_H
