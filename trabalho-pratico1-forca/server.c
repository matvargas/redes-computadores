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
#define LOG 1
#define HINT_FLAG 2
#define OCC_FLAG 3

typedef struct occurrences {
    int occCount;
    int occIndexes[50];
} OccStruct;
;



void usage(int argc, char **argv) {
    //Sugestão de porta: 51511
    printf("usage: %s <server port>\n", argv[0]);
    exit(EXIT_FAILURE);
}

OccStruct testCharOccurrences (char charToTest){

    OccStruct occ;

    memset(occ.occIndexes, -1, 50);
    occ.occCount = 0; 

    if(LOG) printf("[log] testing char: '%c' on word \n", charToTest);

    for(int i = 0; HANGMAN[i]; i++){
        if (HANGMAN[i] == charToTest) {
            if(LOG) printf("[log] Index of occurrence of char: '%c' = %d \n", charToTest, i);
            occ.occIndexes[occ.occCount] = i;
            occ.occCount ++;
        }
    }

    if(LOG) printf("[log] Occurrences of char: '%c' = %d \n", charToTest, occ.occCount);

    return occ;
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

        if(LOG) printf("[msg] Game started \n");

        unsigned long gameCount = sizeof(HANGMAN) - 1;
        unsigned total = 0;
        OccStruct occ;

        count = recv(csock, buf + total, 2, 0);
        total += count;

        if(LOG) printf("[log] received %u bytes\n", total);
        if(LOG) printf("[log] char received: %u \n", buf[1]);

        occ = testCharOccurrences((char)buf[1]);

        int occBuffSize = 2 + occ.occCount;
        if(LOG) printf("[log] size of occurence buffer %d positions \n", occBuffSize);

        unsigned char occBuff[occBuffSize];

        occBuff[0] = (unsigned char)OCC_FLAG;
        if(LOG) printf("[log] inserting %u OCC_FLAG with %lu bytes \n", occBuff[0], sizeof(occBuff[0]));

        occBuff[1] = (unsigned char)occ.occCount;
        if(LOG) printf("[log] inserting %u occurrences with %lu bytes \n", occBuff[1], sizeof(occBuff[1]));

        for(int i = 0; i < occ.occCount; i++) {
            occBuff[i + 2] = (unsigned char)occ.occIndexes[i];
            if(LOG) printf("[log] inserting index %u with %lu bytes \n", occBuff[i + 2], sizeof(occBuff[i + 2]));
        }

        total = 0;
        count = send(csock, occBuff, strlen(occBuff) + 1, 0);
        total += count;

        if(LOG) printf("[log] flag sent: %u \n", occBuff[0]);
        if(LOG) printf("[log] sent %lu bytes\n", sizeof(occBuff));
        
        
        close(csock);
    }

    exit(EXIT_SUCCESS);
}