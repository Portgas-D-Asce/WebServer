#ifndef WEBSERVER_DIRECTORY_HANDLER_H
#define WEBSERVER_DIRECTORY_HANDLER_H
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <cstring>
#include "../http/http.h"
#include "../html/html.h"

class DirectoryHandler {
private:
    std::string _root;
public:
    explicit DirectoryHandler(const std::string& root) : _root(root) {}

    int file_size(const std::string& url) const {
        std::string path = _root + url;
        struct stat info;
        stat(path.c_str(), &info);
        return info.st_size;
    }

    std::pair<char*, size_t> wrapper(std::string path) {
        if(!path.empty() && path.back() == Sen[Sep::SLASH][0]) path.pop_back();

        std::string dir = _root + path;
        DIR* ptr = opendir(dir.c_str());
        struct dirent* ent;

        std::string content;
        if(ptr == nullptr) {
            content = "server busy(maybe fd used up), please wait a moment...";
        } else {
            while((ent = readdir(ptr)) != nullptr) {
                std::string url = path + Sen[Sep::SLASH] + ent->d_name;
                std::string a = Html::a_wrapper(ent->d_name, url);
                int size = file_size(url);
                std::string tds = Html::td_wrapper(a) +
                                  Html::td_wrapper(std::to_string(size));
                content.append(Html::tr_wrapper(tds));
            }
        }
        content = Html::table_wrapper(content);
        content = Html::html_wrapper(path, content);

        std::string header = Http::get_header(Stat::OK, "html", content.size());
        size_t total = header.size() + content.size();
        char* res = (char*)mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memcpy(res, header.c_str(), header.size());
        memcpy(res + header.size(), content.c_str(), content.size());

        return {res, total};
    }

    bool check(const std::string& url) const {
        struct stat info;
        std::string path = _root + url;
        if(stat(path.c_str(), &info) == -1) {
            return false;
        }
        return info.st_mode & S_IFDIR;
    }
};

#endif //WEBSERVER_DIRECTORY_HANDLER_H
