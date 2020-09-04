#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024
#define LOG 1
#define HINT_FLAG 2


void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

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

	char buf[2];
	memset(buf, 0, 2);
	unsigned total = 0;
	
	size_t count = recv(s, buf + total, 2, 0);
	total += count;

	if(LOG) printf("[log] received %u bytes\n", total);
	if(LOG) printf("[msg] Game started, word with %u chars \n", buf[1]);
	printf("%u\n", buf[1]);
	
	int endGame = (int)buf[1];

	char c;
	scanf("%c", &c);
	total = 0;

	while(1) {
		buf[0] = (unsigned char)HINT_FLAG;
		buf[1] = c;

		if(LOG) printf("[log] preparing to send %lu bytes\n", sizeof(buf));
		if(LOG) printf("[log] preparing to send Flag: %u, Char: %c \n", buf[0], buf[1]);
		count = send(s, buf, 2, 0);

		char occBuff[BUFSZ];
		memset(occBuff, 0, BUFSZ);

		count = recv(s, occBuff + total, BUFSZ, 0);
		total += count;	

		
		printf("[log] char %c has %u occurrences\n", c, occBuff[1]);
		printf("[log] occurrences are in: \n");
		for(int i = 0; i < occBuff[1]; i++)
			printf("[%u]", occBuff[i+1]);
		printf("\n");
		
		endGame -= (int)occBuff[1];

		if(LOG) printf("[log] %d chars remaining \n", endGame);

		if (endGame == 0)
			break;

		total = 0;
		scanf("%c", &c);
	}
	
	close(s);

	exit(EXIT_SUCCESS);
}