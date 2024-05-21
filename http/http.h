//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_HTTP_H
#define WEBSERVER_HTTP_H
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "http_header.h"

class Http {
public:
    static std::map<std::string, std::string> parse(const std::string& msg) {
        //printf("%s\nxxxxx\n\n", s.c_str());
        std::map<std::string, std::string> _mp {
            {Pron[Prop::METHOD], ""},
            {Pron[Prop::URL], ""},
            {Pron[Prop::PROTOCOL], ""}
        };

        int n = msg.size(), idx = 0;
        std::string temp;
        while(idx < n && msg[idx] != ' ') {
            temp.push_back(msg[idx++]);
        }
        if(temp != "GET") return _mp;
        _mp[Pron[Prop::METHOD]] = temp;

        temp.clear(); ++idx;
        while(idx < n && msg[idx] != ' ') {
            temp.push_back(msg[idx++]);
        }
        _mp[Pron[Prop::URL]] = temp;

        temp.clear(); ++idx;
        while(idx < n && msg[idx] != '\r') {
            temp.push_back(msg[idx++]);
        }
        _mp[Pron[Prop::PROTOCOL]] = temp;
        return _mp;
    }

    static std::string header_wrapper(Stat code, const std::string& s, std::string suf) {
        std::string t("HTTP/1.0 " + Stan[code] + Sen[Sep::CRLF]);

        t.append(Pron[Prop::SERVER] + ": WebServer/0.0.1" + Sen[Sep::CRLF]);

        if(!Mime.count(suf)) suf = "default";
        t.append(Pron[Prop::CONTENT_TYPE] + ": " + Mime[suf] + Sen[Sep::CRLF]);

        t.append(Pron[Prop::CONTENT_LENGTH] + ": " +
                 std::to_string(s.size()) + Sen[Sep::CRLF]);

        t.append(Sen[Sep::CRLF]);
        t.append(s);

        return t;
    }
};

#endif //WEBSERVER_HTTP_H
