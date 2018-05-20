/* Autor: Piotr Maślankowski
   Numer indeksu: 280354
   Sieci komputerowe 2018
   Pracownia 3 - Transport 
   Plik sliding_window.c */

#include <stdlib.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sliding_window.h"


static window_t window; 
static int sockfd;
static int outfd;
static int port;
static int segments;
static int downloaded_segments;

static int init_window();
static void free_window();
static int send_neccesarry_queries();
static int handle_response();
static int update_window(int msec, int total_size);
static int save_segment(segment_t *seg);


int download(int port, char *output, int size) {
    if(init_window(port, output, size) < 0)
        return -1;
    
    while(window.end > 0) {
        send_neccesarry_queries();

        fd_set descs;
        FD_ZERO(&descs);
        FD_SET(sockfd, &descs);
        struct timeval start_time, end_time;
        
        if(gettimeofday(&start_time, NULL) < 0) {
            fprintf(stderr, "[download] gettimeofdayerror: %s\n", strerror(errno));
            return -1;
        }

        struct timeval tv;
        tv.tv_sec = 2; tv.tv_usec = 0;
        int ready = select(sockfd+1, &descs, NULL, NULL, &tv);
        if(ready < 0) {
            fprintf(stderr, "[download] select error: %s\n", strerror(errno));
            return -1;
        }
        if(ready > 0) {
            if(handle_response() < 0) 
                return -1;
        }

        if(gettimeofday(&end_time, NULL) < 0) {
            fprintf(stderr, "[download] gettimeofdayerror: %s\n", strerror(errno));
            return -1;
        }

        update_window((end_time.tv_sec - start_time.tv_sec) * 1000 + 
            (end_time.tv_usec - start_time.tv_usec) / 1000, size);
    }

    free_window();
    return 0;
}

int init_window(int srv_port, char *output, int size) {
    port = srv_port;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "[init_window]: socket error: %s\n", strerror(errno)); 
		return -1;
	}

    outfd = open(output, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outfd < 0) {
        fprintf(stderr, "[init_window]: fopen error: %s\n", strerror(errno));
        return -1;
    }

    segments = (size+SEGMENT_SIZE-1)/SEGMENT_SIZE;
    int N = WINDOW_SIZE;
    if(segments < N) 
        N = segments;
    window.end = N;
    window.start_segment = 0;
    for(int i=0; i < N+1; i++) { // last element = guard
        window.segments[i] = (segment_t*) malloc(sizeof(segment_t));
        window.segments[i]->start = i * SEGMENT_SIZE;
        window.segments[i]->elapsed_time = 0;
        window.segments[i]->status = NOT_DOWNLOADED;
        if(i == size / SEGMENT_SIZE)
            window.segments[i]->size = size % SEGMENT_SIZE;
        else
            window.segments[i]->size = SEGMENT_SIZE;
    }

    return 0;
}

void free_window() {
    for(int i=0; i <= window.end; i++)
        free(window.segments[i]);
    close(sockfd);
    close(outfd);
}

int update_window(int msec, int total_size) {
    for(int i=0; i < window.end; i++) 
        window.segments[i]->elapsed_time += msec;
    
    int index;
    for(index=0; index <= window.end; index++) {
        if(window.segments[index]->status == NOT_DOWNLOADED)
            break;
        else if (window.segments[index]->status == DOWNLOADED) {
            if(save_segment(window.segments[index]) < 0)
                return -1;
            window.segments[index]->status = SAVED;
        }
    }

    if(index == 0) //nothing to change, window should not be slided
        return 0;
    
    for(int i=0; i < index; i++) 
        free(window.segments[i]);
    for(int i=index; i <= window.end; i++)
        window.segments[i-index] = window.segments[i];

    window.end -= index;
    window.start_segment += index;
    if(window.end == 0)
        return 0;
    
    while(window.end < WINDOW_SIZE && 
        window.segments[window.end-1]->start / SEGMENT_SIZE < total_size / SEGMENT_SIZE) {
        window.segments[window.end+1] = window.segments[window.end]; // moving guard 
        window.segments[window.end] = (segment_t*) malloc(sizeof(segment_t));
        window.segments[window.end]->elapsed_time = 0;
        window.segments[window.end]->start = window.segments[window.end-1]->start + SEGMENT_SIZE;
        if(window.segments[window.end]->start / SEGMENT_SIZE == total_size / SEGMENT_SIZE) // last segment
            window.segments[window.end]->size = total_size % SEGMENT_SIZE;
        else
            window.segments[window.end]->size = SEGMENT_SIZE;
        window.segments[window.end]->status = NOT_DOWNLOADED;
        window.end++;
    }
    return 0;
}

int save_segment(segment_t *seg) {
    ssize_t written = write(outfd, seg->data, seg->size);
    if(written < 0) {
        fprintf(stderr, "[save_segment] write error: %s\n", strerror(errno));
        return -1;
    }
    if (written != seg->size) {
        fprintf(stderr, "[save_segment] error during writing to output file: bytes to save: %d, bytes saved: %ld\n", seg->size, written);
        return -1;
    }
    return 0;
}

int send_neccesarry_queries() {
    struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &server_address.sin_addr);

    for(int i=0; i < window.end; i++) {        
        if(window.segments[i]->status == NOT_DOWNLOADED && window.segments[i]->elapsed_time >= RESPONSE_TIMEOUT) {
            window.segments[i]->elapsed_time = 0;
            
            char message[40];
            sprintf(message, "GET %d %d\n", 
                window.segments[i]->start, 
                window.segments[i]->size);
	        ssize_t message_len = strlen(message);
            for(int j=0; j < QUERIES_PER_SEGMENT; j++) {
	            if (sendto(sockfd, message, message_len, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != message_len) {
		            fprintf(stderr, "[send_neccesary_queries] sendto error: %s\n", strerror(errno)); 
                    return -1;		
                }
            }
        }
    }

    return 0;
}

int handle_response() {
    struct sockaddr_in  sender;	
    socklen_t           sender_len = sizeof(sender);
    char                buffer[IP_MAXPACKET+1];

    ssize_t datagram_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
    if (datagram_len < 0) {
        fprintf(stderr, "[handle_response]: recvfrom error: %s\n", strerror(errno)); 
        return -1;
    }

    char sender_ip_str[20]; 
	inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
    int recv_port = ntohs(sender.sin_port);
    if(strcmp(sender_ip_str, SERVER_ADDR) == 0 && recv_port == port) {
	    buffer[datagram_len] = 0;
        int start, size;
        sscanf(buffer, "DATA %d %d\n", &start, &size);
        char *data = strchr(buffer, '\n') + 1;
        

        int window_index = (start / SEGMENT_SIZE) - window.start_segment;
        if(0 <= window_index && window_index < window.end) {
            segment_t *seg = window.segments[window_index];
            assert(seg->start == start);
            assert(seg->size == size);
            if(seg->status == NOT_DOWNLOADED) {
                memcpy(seg->data, data, size);
                seg->status = DOWNLOADED;
                if(size>0) { // last packet has 0 size
                    downloaded_segments++;
                    printf("%.2f%% done\n", (double) downloaded_segments / segments * 100.0);
                }
            }
        }
    }
    return 0;
}