#ifndef _HTTP_REQUEST_
#define _HTTP_REQUEST_

#include <string>


class http_request {
public:
    http_request(char *bytes, int size);

    bool is_valid() const { return valid; }
    bool should_close_immediately() const { return close_immediately; }

    void handle(int fd, int port, const std::string& directory);

private:

    std::string response_301(int port, const std::string& directory) { 
        std::string result = "HTTP/1.1 301 Moved Permanently\r\n";
        result += "Location: http://" + host + ":" + std::to_string(port) + "/" + directory + "/index.html\r\n"; 
        result += "Connection: Keep-Alive\r\n";
        return result;
    }

    std::string response_403() {
        std::string result = "HTTP/1.1 403 Forbidden\r\n";
        result += "Content-Length: 78\r\n";
        result += "\r\n";
        result += "<html><head><title>Error 403</title><body><h1>403 Forbidden</h1></body></html>";
        return result;
    }

    std::string response_404() {
        std::string result = "HTTP/1.1 404 Not Found\r\n";
        result += "Content-Length: 78\r\n";
        result += "\r\n";
        result += "<html><head><title>Error 404</title><body><h1>404 Not Found</h1></body></html>";
        return result;
    }

    std::string response_501() {
        std::string result = "HTTP/1.1 501 Not Implemented\r\n";
        result += "Content-Length: 84\r\n";
        result += "\r\n";
        result += "<html><head><title>Error 501</title><body><h1>501 Not Implemented</h1></body></html>";
        return result;
    }

    void send_response_200(int fd, const std::string& path);
    std::string get_content_type(const std::string& path);
    void send_response(int fd, const char *data, size_t size);

    static bool ends_with(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }

    static bool is_unsafe(const char *path, const char *host, const char *server_directory);

private:
    bool valid;
    bool close_immediately;
    std::string path;
    std::string host;
};

#endif