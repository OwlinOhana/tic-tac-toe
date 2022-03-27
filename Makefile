all:
	g++ -Wall -g -O0 server.cpp serverMain.cpp clieserv.c -o server -pthread 
	gcc -Wall -g -O0 client.c clientMain.c clieserv.c -o client 

clean: 
	rm -rf *.o  