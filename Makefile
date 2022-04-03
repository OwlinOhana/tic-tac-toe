all:
	g++ -Wall -g -O0 server.cpp -o server -lpthread 
	g++ -Wall -g -O0 client.cpp -o client 

clean: 
	rm -rf *.o 