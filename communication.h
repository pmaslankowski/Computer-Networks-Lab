/* Computer Networks 2018
   University of Wrocław
   Laboratory 1 - Traceroute
   Author: Piotr Maślankowski */

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>


typedef struct icmp_response {
    char ip[20];
    pid_t pid;
    int ttl;
    int type;
} icmp_response_t;


int open_icmp_socket();
ssize_t send_icmp_echo_request(int sockfd, char *addr, pid_t pid, int ttl);
ssize_t wait_and_receive_data(int sockfd, struct timeval *remaining_time, struct timeval *receive_time, icmp_response_t *response);


#endif