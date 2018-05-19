#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "http_request.h"

#define THROW_ERROR(desc) throw std::runtime_error(std::string(desc) + ": " + std::string(strerror(errno)))
#define THROW_CUSTOM_ERROR(desc) throw std::runtime_error(std::string(desc))

http_request::http_request(char *bytes, int bytes_size) {
    if(strncmp(bytes, "GET", 3) != 0) {
        valid = false;
        return;
    }

    valid = true;
    int idx = 4;
    while(idx < bytes_size && bytes[idx] != ' ' && bytes[idx] != '\r')
        path += bytes[idx++];
    
    char *conn_str = strstr(bytes, "Connection:");
    if(conn_str != nullptr && strncmp(conn_str, "Connection: close", 17) == 0)
        close_immediately = true;
    else
        close_immediately = false;
    
    char *host_str = strstr(bytes, "Host:");
    int offset = host_str - bytes;
    if(host_str != nullptr) {
        int idx = 6;
        while(offset + idx < bytes_size && host_str[idx] != '\r' && host_str[idx] != ':')
            host += host_str[idx++];
    }
}


void http_request::handle(int fd, int port, const std::string& directory) {
    if(!valid) {
        std::string response = response_501();
        send_response(fd, response.c_str(), response.size()); 
        return;
    }

    struct stat sb;
    std::string full_path = directory + "/" + host + path;
    if(stat(full_path.c_str(), &sb) != 0) {
        if(errno == ENOENT) {
            std::string response = response_404();
            send_response(fd, response.c_str(), response.size());
        } else
            THROW_ERROR("stat error");
        return;
    }
    
    if(S_ISDIR(sb.st_mode)) {
        printf("[info] Redirecting to index...\n");
        std::string response = response_301(port, directory);
        send_response(fd, response.c_str(), response.size());
        return;
    }

    if(is_unsafe(path.c_str(), host.c_str(), directory.c_str())) {
        printf("[info] Attempt of illegal access\n");
        std::string response = response_403();
        send_response(fd, response.c_str(), response.size());
        return;
    }
    printf("[Info] Sending file: %s ...\n", full_path.c_str());
    send_response_200(fd, full_path);
}


void http_request::send_response_200(int fd, const std::string& path) {
    std::string result_buff = "HTTP/1.1 200 OK\r\n";
    FILE *f = fopen(path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fclose(f);

    result_buff += "Content-Length: " + std::to_string(len) + "\r\n";
    result_buff += "Content-Type: " + get_content_type(path) + "\r\n";
    result_buff += "Connection: Keep-Alive\r\n";
    result_buff += "\r\n";
    send_response(fd, result_buff.c_str(), result_buff.size());

    int infd = open(path.c_str(), O_RDONLY);
    if(infd < 0)
        THROW_ERROR("open error");
    sendfile(fd, infd, NULL, len);
    close(infd);
}


std::string http_request::get_content_type(const std::string& path) {
    if(ends_with(path, ".txt"))
        return "text/plain";
    if(ends_with(path, ".html"))
        return "text/html";
    if(ends_with(path, ".css"))
        return "text/css";
    if(ends_with(path, ".jpg"))
        return "image/jpeg";
    if(ends_with(path, ".jpeg"))
        return "image/jpeg";
    if(ends_with(path, ".png"))
        return "image/png";
    if(ends_with(path, ".pdf"))
        return "application/pdf";
    return "application/octet-stream";
}


void http_request::send_response(int fd, const char* data, size_t size) {
    if(send(fd, data, size, 0) < 0)
        THROW_ERROR("send error");
}

bool http_request::is_unsafe(const char *path, const char *host, const char *server_directory) {
  char real_filepath[100];
  char cwd[100];
  char domain[100];
  realpath(path, real_filepath);
  getcwd(cwd, 100);

  strcat(cwd, "/");
  strcat(cwd, server_directory);

  int i = 0;
  for(i = 0; i < 100; i++)
    if (cwd[i] != real_filepath[i])
      break;
  i++;

  strcpy(domain, real_filepath + i);
  char * slash_ptr = strchr(domain, '/');
  if (slash_ptr == NULL)
    return false;

  *slash_ptr = '\0';

  return (strcmp(domain, host) != 0);
}