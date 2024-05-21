#ifndef WEBSERVER_HTML_H
#define WEBSERVER_HTML_H
#include <string>

class Html {
public:
    static std::string html_wrapper(const std::string& title, const std::string& labels) {
        std::string html("<html><head><title>");
        html.append(title);
        html.append("</title></head><body>");
        html.append(labels);
        html.append("</body></html>");
        return html;
    }

    static std::string a_wrapper(const std::string& txt, const std::string& href) {
        return "<a href=\"" + href + "\">" + txt + "</a>";
    }

    static std::string div_wrapper(const std::string& txt) {
        return "<div>" + txt + "</div>";
    }

    static std::string tr_wrapper(const std::string& txt) {
        return "<tr>" + txt + "</tr>";
    }

    static std::string td_wrapper(const std::string& txt) {
        return "<td>" + txt + "</td>";
    }

    static std::string table_wrapper(const std::string& txt) {
        return "<table>" + txt + "</table>";
    }
};


#endif //WEBSERVER_HTML_H
