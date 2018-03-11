/* Computer Networks 2018
   University of Wrocław
   Laboratory 1 - Traceroute
   Author: Piotr Maślankowski */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "communication.h"
#include "utility.h"


static void get_address_from_arguments(int argc, char **argv, char *addr);


int main(int argc, char **argv) {
    char addr[20];
    get_address_from_arguments(argc, argv, addr);

    int sockfd = open_icmp_socket();
    pid_t pid = getpid();
    
    for(int ttl=1; ttl <= 30; ttl++) {

        for(int i=0; i < 3; i++)
            send_icmp_echo_request(sockfd, addr, ttl, pid);

        icmp_response_t response[3];
        struct timeval times[3];

        struct timeval current_time, end_time, start_time, remaining_time, receive_time;
        get_current_time(&current_time);
        get_current_time(&start_time);
        end_time.tv_sec = current_time.tv_sec + 1;
        end_time.tv_usec = current_time.tv_usec;
        remaining_time.tv_sec = 1;
        remaining_time.tv_usec = 0;

        int responses = 0;
        while(!timeval_subtract(&remaining_time, &end_time, &current_time) && responses < 3) {
            wait_and_receive_data(sockfd, &remaining_time, &receive_time, &response[responses]);
            timeval_subtract(&times[responses], &receive_time, &start_time);
            if(response[responses].ttl == ttl && response[responses].pid == pid)
                responses++;
            get_current_time(&current_time);
        }

        print_traceroute_line(ttl, response, responses, times);

        for(int i=0; i < responses; i++)
            if(response[i].type == ICMP_ECHOREPLY)
                return 0;
    }

    return 0;
}


static void get_address_from_arguments(int argc, char **argv, char *addr) {
    if (argc != 2) {
        printf("Invalid number of program arguments. 1 argument - IP address should be provided.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(addr, argv[1]);
    if(!is_valid_addr(addr)) {
        printf("Invalid program argument. %s is not valid IP address\n", addr);
        exit(EXIT_FAILURE);
    }
}