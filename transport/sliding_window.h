#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

/* Autor: Piotr Maślankowski
   Numer indeksu: 280354
   Sieci komputerowe 2018
   Pracownia 3 - Transport 
   Plik sliding_window.h */

#include <stdint.h>

#define SEGMENT_SIZE 1000
#define WINDOW_SIZE 2000
#define RESPONSE_TIMEOUT 1
#define QUERIES_PER_SEGMENT 3

#define SERVER_ADDR "156.17.4.30"
#define SERVER_PORT 40001

typedef enum status { NOT_DOWNLOADED, DOWNLOADED, SAVED } status_t;

typedef struct segment {
    int elapsed_time;
    int start;
    int size;
    status_t status;
    char data[SEGMENT_SIZE];
} segment_t;

typedef struct window {
    segment_t *segments[WINDOW_SIZE+1];
    int end;
    int start_segment;
} window_t;

int download(int port, char *output, int size);

#endif