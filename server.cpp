#include <cstdio>
#include "http_server.h"

using namespace std;

int main(int argc, char **argv) {
    printf("Pamiętaj, że Firefox robi domyślnie do 6 połączeń, więc strony się wolno ładują. Zmiana: about:config, max connections per server\n");
    if(argc != 3) {
        printf("usage:\n\t./server port directory\n");
        return 0;
    }
    
    int port;
    if(sscanf(argv[1], "%d", &port) < 0) {
        printf("usage:\n\t./server port directory\n");
        return 0;    
    }

    char path[100];
    if(sscanf(argv[2], "%s", path) < 0) {
        printf("usage:\n\t./server port directory\n");
        return 0;      
    }

    http_server server { port, path };
    server.run();
    return 0;
}