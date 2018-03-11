all: icmp_receive traceroute

icmp_receive: icmp_receive.c
	gcc -std=gnu11 -Wall -Wextra -g icmp_receive.c -o icmp_receive

traceroute: traceroute.c communication.c communication.h utility.c utility.h
	gcc -std=gnu11 -Wall -Wextra -g traceroute.c communication.c utility.c -o traceroute

clean:
	rm icmp_receive