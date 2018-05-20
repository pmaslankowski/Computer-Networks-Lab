/* Autor: Piotr Maślankowski
   Numer indeksu: 280354
   Sieci komputerowe 2018
   Pracownia 3 - Transport 
   Plik transport.c */

#include <stdio.h>
#include "sliding_window.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("usage: ./transport port filename size\n");
        return 0;
    }

    int port;
    if(sscanf(argv[1], "%d", &port) == 0) {
        printf("usage: ./transport port filename size\n");
        return 0;    
    }

    char filename[250];
    if(sscanf(argv[2], "%s", filename) == 0) {
        printf("usage: ./transport port filename size\n");
        return 0;
    }

    int size;
    if(sscanf(argv[3], "%d", &size) == 0) {
        printf("usage: ./transport port filename size\n");
        return 0;
    }

    download(port, filename, size);

    return 0;
}