/* Computer Networks 2018
   University of Wrocław
   Laboratory 1 - Traceroute
   Author: Piotr Maślankowski */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "utility.h"
#include "communication.h"


static double average(struct timeval *time, int count);


void get_current_time(struct timeval *tv) {
    if(gettimeofday(tv, NULL) == -1) {
        printf("gettimeofday: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


// function "timeval_subtract" copied from http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y) {
    /* Perform the carry for the later subtraction by updating y. */
    if(x->tv_usec < y->tv_usec){
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if(x->tv_usec - y->tv_usec > 1000000){
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}


void print_traceroute_line(int ttl, icmp_response_t *response, int responses_count, struct timeval *time) {
    if(responses_count == 0) {
        printf("%-6d%-18s %-4s\n", ttl, "*", "???");
        return;
    }

    printf("%-6d%-18s ", ttl, response[0].ip);
    icmp_response_t *prev_response = &response[0];
    for(int i=1; i < responses_count; i++) {
        if(strcmp(prev_response->ip, response[i].ip) == 0) continue;
        printf("%s ", response[i].ip);
        prev_response = &response[i];
    }
    if(responses_count != 3)
        printf("%-6s\n", "???");
    else
        printf("%-4.2fms\n", average(time, responses_count));
}


int is_valid_addr(char* addr) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, addr, &(sa.sin_addr));
    return result != 0;
}


static double average(struct timeval *time, int count) {
    double res = 0;
    for(int i=0; i < count; i++)
        res += (double)time[i].tv_usec / 1000;
    return res / count;
}
