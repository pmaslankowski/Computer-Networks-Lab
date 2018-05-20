all: transport

transport: transport.o sliding_window.o
	gcc -std=gnu99 -Wall -Wextra transport.o sliding_window.o -o transport

transport.o: transport.c 
	gcc -std=gnu99 -Wall -Wextra -c -g  transport.c

sliding_window.o: sliding_window.c sliding_window.h
	gcc -std=gnu99 -Wall -Wextra -c -g sliding_window.c
 
clean:
	rm *.o

distclean:
	rm transport
