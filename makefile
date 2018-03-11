all: icmp_receive

icmp_receive: icmp_receive.c
	gcc -std=gnu11 -Wall -Wextra -g icmp_receive.c -o icmp_receive

clean:
	rm icmp_receive