/* Computer Networks 2018
   University of Wrocław
   Laboratory 1 - Traceroute
   Author: Piotr Maślankowski */

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <sys/time.h>

typedef struct icmp_response icmp_response_t;

void get_current_time(struct timeval *tv);
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
void print_traceroute_line(int ttl, icmp_response_t *response, int responses_count, struct timeval *time);
int is_valid_addr(char *addr);

#endif 