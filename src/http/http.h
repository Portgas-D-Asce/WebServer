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
        std::map<std::string, std::string> _mp {
            {Pron[Prop::METHOD], ""},
            {Pron[Prop::URL], ""},
            {Pron[Prop::PROTOCOL], ""}
        };

        int n = msg.size(), idx = 0;
        std::string temp;
        while(idx < n && msg[idx] != Sen[Sep::SPACE][0]) {
            temp.push_back(msg[idx++]);
        }
        if(temp != Metn[Meth::GET]) return _mp;
        _mp[Pron[Prop::METHOD]] = temp;

        temp.clear(); ++idx;
        while(idx < n && msg[idx] != Sen[Sep::SPACE][0]) {
            temp.push_back(msg[idx++]);
        }
        _mp[Pron[Prop::URL]] = _decode(temp);

        temp.clear(); ++idx;
        while(idx < n && msg[idx] != Sen[Sep::CRLF][0]) {
            temp.push_back(msg[idx++]);
        }
        _mp[Pron[Prop::PROTOCOL]] = temp;
        return _mp;
    }

    static std::string get_header(Stat code, std::string suf, size_t length) {
        std::string t(Ven[Ver::HTTP_1_0] + Sen[Sep::SPACE] + Stan[code] + Sen[Sep::CRLF]);

        t.append(Pron[Prop::SERVER] + ": WebServer/0.0.1" + Sen[Sep::CRLF]);

        if(!Mime.count(suf)) suf = "default";
        t.append(Pron[Prop::CONTENT_TYPE] + Sen[Sep::COLON] +
                 Mime[suf] + Sen[Sep::CRLF]);

        t.append(Pron[Prop::CONNECTION] + Sen[Sep::COLON] +
                 "keep-alive" + Sen[Sep::CRLF]);

        t.append(Pron[Prop::CONTENT_LENGTH] + Sen[Sep::COLON] +
                 std::to_string(length) + Sen[Sep::CRLF]);

        t.append(Sen[Sep::CRLF]);

        return t;
    }
private:
    static std::string _decode(const std::string& s) {
        int n = s.size();
        std::string t;
        for(int i = 0; i < n; ++i) {
            if(s[i] == '%') {
                t.push_back(_code(s[i + 1]) * 16 + _code(s[i + 2]));
                i += 2;
            } else {
                t.push_back(s[i]);
            }
        }
        return t;
    }

    static int _code(char ch) {
        if(isdigit(ch)) {
            return ch - '0';
        }
        ch |= 1 << 5;
        return ch - 'a';
    }
};

#endif //WEBSERVER_HTTP_H
