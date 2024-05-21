//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_HTTP_HEADER_H
#define WEBSERVER_HTTP_HEADER_H
#include <string>
#include <map>

enum Prop {
    METHOD,
    PROTOCOL,
    URL,
    CODE,
    SERVER,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    PROP_END
};

const std::string Pron[PROP_END] = {
    "Method",
    "Protocol",
    "Url",
    "Code",
    "Server",
    "Content-Type",
    "Content-Length"
};

enum Stat {
    OK,
    NOT_FOUND,
    STAT_END
};

const std::string Stan[STAT_END] = {
    "200 OK",
    "400 NOT FOUND"
};

enum Sep {
    CRLF,
    SEP_END
};

const std::string Sen[SEP_END] = {
    "\r\n"
};

std::map<std::string, std::string> Mime = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"png", "image/png"},
    {"css", "text/css"},
    {"au", "audio/basic"},
    {"wav", "audio/wav"},
    {"avi", "video/x-msvideo"},
    {"mov", "video/quicktime"},
    {"qt", "video/quicktime"},
    {"mpeg", "video/mpeg"},
    {"mpe", "video/mpeg"},
    {"vrml", "model/vrml"},
    {"wrl", "model/vrml"},
    {"midi", "audio/midi"},
    {"mid", "audio/midi"},
    {"mp3", "audio/mpeg"},
    {"ogg", "application/ogg"},
    {"pac", "application/x-ns-proxy-autoconfig"},
    {"default", "text/plain; charset=utf-8"}
};


#endif //WEBSERVER_HTTP_HEADER_H
