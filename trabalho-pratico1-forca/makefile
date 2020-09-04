all:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -o server

server:
	gcc -Wall -c common.c
	gcc -Wall server.c common.o -o server

client:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client

clean:
	rm common.o client server server-mt                                  