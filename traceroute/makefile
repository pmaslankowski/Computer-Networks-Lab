all: traceroute

traceroute: traceroute.o communication.o utility.o
	gcc -std=gnu99 -Wall -Wextra traceroute.o communication.o utility.o -o traceroute

traceroute.o: traceroute.c 
	gcc -std=gnu99 -Wall -Wextra -c traceroute.c communication.c utility.c

communication.o: communication.c communication.h
	gcc -std=gnu99 -Wall -Wextra -c communication.c

utility.o: utility.c utility.h
	gcc -std=gnu99 -Wall -Wextra -c utility.c

clean:
	rm *.o

distclean:
	rm traceroute
