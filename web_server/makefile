all: server

server: server.o http_server.o http_request.o
	g++ -pthread server.o http_server.o http_request.o -o server

server.o: server.cpp server.h
	g++ -c -g server.cpp -std=c++11 -Wall -Wextra

http_server.o: http_server.cpp http_server.h 
	g++ -c -g http_server.cpp -std=c++11 -Wall -Wextra

http_request.o: http_request.cpp http_request.h
	g++ -c -g http_request.cpp -std=c++11 -Wall -Wextra

clean: 
	rm *.o

distclean:
	rm server
