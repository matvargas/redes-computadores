#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	// char buf[BUFSZ];
	// memset(buf, 0, BUFSZ);
	// printf("mensagem> ");
	// fgets(buf, BUFSZ-1, stdin);

	// char *p;
	// if(p=strchr(buf, '\n')){
	// 	*p = 0;
	// } else {
	// 	scanf("%*[^\n]");scanf("%*c");
	// }

	// size_t count = send(s, buf, strlen(buf)+1, 0);
	// if (count != strlen(buf)+1) {
	// 	logexit("send");
	// }

	char buf[2];
	memset(buf, 0, 2);
	unsigned total = 0;
	
		size_t count = recv(s, buf + total, 2, 0);
		total += count;
	
	close(s);

	printf("received %u bytes\n", total);
	for(int i = 0; i < sizeof(buf); i++)
		printf("%u", buf[i]);

	exit(EXIT_SUCCESS);
}