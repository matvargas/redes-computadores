#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define IP_VERSION "v4"
#define HANGMAN "pneumoultramicroscopicossilicovulcanoconiotico"

void usage(int argc, char **argv) {
    //Sugestão de porta: 51511
    printf("usage: %s <server port>\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
 
    if (0 != server_sockaddr_init(IP_VERSION, argv[1], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        unsigned char buf[2];
        memset(buf, 0, 2);
        
        unsigned char startGame = 1;
        unsigned char wordLen = sizeof(HANGMAN) - 1;
        buf[0] = startGame;
        buf[1] = wordLen;

        size_t count = send(csock, buf, 2, 0);
        if (count != 2) {
            logexit("send");
        }

        
        count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        close(csock);
    }

    exit(EXIT_SUCCESS);
}