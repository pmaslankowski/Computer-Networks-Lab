all: server

server: server.o 
	g++ server.o -o server

server.o: server.cpp server.h
	g++ -c server.cpp -std=c++11 -Wall -Wextra

clean: 
	rm *.o

distclean:
	rm server
