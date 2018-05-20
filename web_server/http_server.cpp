#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <vector>
#include <thread>
#include "http_server.h"
#include "http_request.h"

#define THROW_ERROR(desc) throw std::runtime_error(std::string(desc) + ": " + std::string(strerror(errno)))
#define THROW_CUSTOM_ERROR(desc) throw std::runtime_error(std::string(desc))


http_server::http_server(int port, const std::string& directory)
    : port(port), directory(directory) {
    struct stat sb;
    if(stat(directory.c_str(), &sb) != 0)
        THROW_ERROR("stat error");
    if(!S_ISDIR(sb.st_mode))
        THROW_CUSTOM_ERROR("Directory path is not a directory.");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        THROW_ERROR("socket error");
    
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (const sockaddr*) &server_addr, sizeof(server_addr)) < 0)
        THROW_ERROR("bind error");
}


void http_server::run() {
    if(listen(sockfd, 64) < 0)
        THROW_ERROR("listen error");
    
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int conn_sockfd = accept(sockfd, (sockaddr*) &client_addr, &len);
        if(conn_sockfd < 0)
            THROW_ERROR("accept error");
        
        char ip_addr[20];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, sizeof(ip_addr));
        printf("[Info] Client %s:%d connected\n", ip_addr, ntohs(client_addr.sin_port));

        struct timeval prev_tv, now_tv;
        if(gettimeofday(&prev_tv, nullptr) < 0)
            THROW_ERROR("gettimeofday");

        do {
            char recv_buffer[BUFFER_SIZE+1];
            printf("[Info] Listening for packets...\n");
            int received_size = receive_data(conn_sockfd, recv_buffer, BUFFER_SIZE, CONNECTION_TIMEOUT);
            recv_buffer[received_size+1] = 0;
            if(received_size > 0) {
                http_request req {recv_buffer, received_size};
                req.handle(conn_sockfd, port, directory);
            
                if(req.should_close_immediately())
                    break;
                
                if(gettimeofday(&prev_tv, nullptr) < 0)
                    THROW_ERROR("gettimeofday");
            }

            if(gettimeofday(&now_tv, nullptr) < 0)
                THROW_ERROR("gettimeofday");

        } while(now_tv.tv_sec - prev_tv.tv_sec + (now_tv.tv_usec - prev_tv.tv_usec) * 1.0 / 1000000 < CONNECTION_TIMEOUT);
        printf("[info] Connection timeout\n");
        close(conn_sockfd);
    }
}


int http_server::receive_data(int fd, char *buffer, int buffer_size, int timeout) {
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(fd, &descriptors);
    int ready = select(fd+1, &descriptors, nullptr, nullptr, &tv);
    if(ready < 0)
        THROW_ERROR("select error");
    if(ready == 0)
        return -1; //timeout
    int bytes_read = recv(fd, buffer, buffer_size, 0);
    if (bytes_read < 0)
        THROW_ERROR("recv error");

    return bytes_read;
}
