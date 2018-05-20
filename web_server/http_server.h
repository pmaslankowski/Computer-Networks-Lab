#ifndef _HTTP_SERVER_
#define _HTTP_SERVER_

#include <string>

const int BUFFER_SIZE = 1000;
const int CONNECTION_TIMEOUT = 1;

class http_server {
public:
    http_server(int port, const std::string& directory);
    void run();

private:
    int receive_data(int fd, char *buffer, int buffer_size, int timeout);
    
private:
    int port;
    int sockfd;    
    std::string directory;
};

#endif