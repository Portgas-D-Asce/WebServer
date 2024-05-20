//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_HTTP_REQUEST_H
#define WEBSERVER_HTTP_REQUEST_H
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>

class HttpRequest {
private:
    std::map<std::string, std::string> _mp;
public:
    bool parse(const std::string& msg) {
        //printf("%s\nxxxxx\n\n", s.c_str());
        int n = msg.size(), idx = 0;
        std::string temp;
        while(idx < n && msg[idx] != ' ') {
            temp.push_back(msg[idx++]);
        }
        if(temp != "GET") return false;
        _mp["method"] = temp;

        temp.clear();
        while(++idx < n && msg[idx] != ' ') {
            temp.push_back(msg[idx++]);
        }
        _mp["url"] = temp;

        temp.clear();
        while(++idx < n && msg[idx] != '\r') {
            temp.push_back(msg[idx++]);
        }
        _mp["protocol"] = temp;
        return true;
    }

    std::string url() {
        return _mp["url"];
    }
};


#endif //WEBSERVER_HTTP_REQUEST_H
