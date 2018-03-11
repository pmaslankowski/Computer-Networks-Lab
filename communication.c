/* Computer Networks 2018
   University of Wrocław
   Laboratory 1 - Traceroute
   Author: Piotr Maślankowski */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include "communication.h"
#include "utility.h"



static void prepare_icmp_header(struct icmphdr *header, pid_t pid, int current_ttl);
static void prepare_addr_struct(struct sockaddr_in *recipient, char *addr);
static void set_socket_ttl(int sockfd, int ttl);
static u_int16_t compute_icmp_checksum(const void *buff, int length);
static int receive_data(int sockfd, icmp_response_t *response, struct timeval *receive_time);
static void parse_ip_addr(struct sockaddr_in *sender, char *ip_addr, ssize_t ip_addr_size);
static int get_icmp_packet_type(u_int8_t *buffer);
static struct icmphdr *get_icmp_header(u_int8_t *datagram_addr);
static void parse_echoreply_packet(u_int8_t *buffer, icmp_response_t *response);
static void parse_timeexceeded_packet(u_int8_t *buffer, icmp_response_t *response);



int open_icmp_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd == -1)
        printf("open_icmp_socket/socket: %s\n", strerror(errno));
    return sockfd;
}


ssize_t send_icmp_echo_request(int sockfd, char *addr, int ttl, pid_t pid) {
    struct icmphdr icmp_header;
    prepare_icmp_header(&icmp_header, pid, ttl);

    struct sockaddr_in recipient;
    prepare_addr_struct(&recipient, addr);
    
    set_socket_ttl(sockfd, ttl);

    return sendto(sockfd, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
}


ssize_t wait_and_receive_data(int sockfd, struct timeval *remaining_timeval, struct timeval *receive_time, icmp_response_t *response) {
	fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(sockfd, &descriptors);
    int ready = select(sockfd+1, &descriptors, NULL, NULL, remaining_timeval);
    
    if(ready < 0) {
        printf("wait_and_receive_data/select: %s\n", strerror(errno));
        return -1;
    }

    if (ready == 0)
        return 0;
    
    return receive_data(sockfd, response, receive_time);
}


static int receive_data(int sockfd, icmp_response_t *response, struct timeval *receive_time) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];
	ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, MSG_DONTWAIT, (struct sockaddr*)&sender, &sender_len);
	
    get_current_time(receive_time);

    if (packet_len < 0) {
		fprintf(stderr, "receive_data/recvfrom: %s\n", strerror(errno));
        return -1;
    }
    
    parse_ip_addr(&sender, response->ip, sizeof(response->ip));
    response->type = get_icmp_packet_type(buffer);
    if(response->type == ICMP_ECHOREPLY)
        parse_echoreply_packet(buffer, response);
    else if(response->type == ICMP_TIMXCEED)
        parse_timeexceeded_packet(buffer, response);
    else {
        response->ttl = -1; //it can be ICMP packet for another process
        return 1;
    }

    return 1;
} 


static int get_icmp_packet_type(u_int8_t *buffer) {
    struct icmphdr* icmp_header = get_icmp_header(buffer);
    return icmp_header->type;    
}


static void parse_ip_addr(struct sockaddr_in *sender, char *ip_addr, ssize_t ip_addr_size) {
    inet_ntop(AF_INET, &(sender->sin_addr), ip_addr, ip_addr_size);
}


static void parse_echoreply_packet(u_int8_t *buffer, icmp_response_t *response) {
    struct icmphdr* icmp_header = get_icmp_header(buffer);
    response->ttl = icmp_header->un.echo.sequence;
    response->pid = icmp_header->un.echo.id;       
}


static void parse_timeexceeded_packet(u_int8_t *buffer, icmp_response_t *response) {
    struct icmphdr *icmp_header = get_icmp_header((u_int8_t*)get_icmp_header(buffer) + 8); // offset 8 bytes because of orginal ip header in timeexceeded packet
    response->ttl = icmp_header->un.echo.sequence;
    response->pid = icmp_header->un.echo.id;   
}


static struct icmphdr* get_icmp_header(u_int8_t *datagram_addr) {
    struct iphdr* ip_header = (struct iphdr*) datagram_addr;
    u_int8_t* icmp_packet = datagram_addr + 4 * ip_header->ihl;
    return (struct icmphdr*) icmp_packet;
}


static void prepare_icmp_header(struct icmphdr *header, pid_t pid, int current_ttl) {
    header->type = ICMP_ECHO;
    header->code = 0;
    header->un.echo.id = pid;
    header->un.echo.sequence = current_ttl;
    header->checksum = 0;
    header->checksum = compute_icmp_checksum ((u_int16_t*)header, sizeof(header));
}


static void prepare_addr_struct(struct sockaddr_in *recipient, char *addr) {
    bzero (recipient, sizeof(*recipient));
    recipient->sin_family = AF_INET;
    inet_pton(AF_INET, addr, &recipient->sin_addr);
}


static void set_socket_ttl(int sockfd, int ttl) {
    if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) == -1)
        printf("set_socket_ttl/setsockopt: %s", strerror(errno));
}


static u_int16_t compute_icmp_checksum(const void *buff, int length) {
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}