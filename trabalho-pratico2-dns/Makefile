all:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall async-server.c -lpthread -o servidor-dns

clean:
	rm common.o client server servidor-dns
