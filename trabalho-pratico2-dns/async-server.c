/*
    Compile
    gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> //for threading

typedef struct host{
    char *hostName;
    char *ip;
} host;

int hostCount = 0;
host *hosts[1000];

void *connection_handler(void *);
void initServerConfigs(char *configFile);
void cmdBuilder(char *readBuffer, int charCount);
int addHostName(host *h);
void listHosts();
host *createHost(char *ip, char *hostName);

int main(int argc, char *argv[]) {

    // pthread_t thread_id;
    char buffer[254];

    if (argc > 2) {
        initServerConfigs(argv[2]);
    }
    
    int port = atoi(argv[1]);
    // // create thread for socket 
    // if (pthread_create(&thread_id, NULL, connection_handler, (void*) port) < 0) {
    //     perror("could not create thread");
    //     return 1;
    // }

    // continue listening to stdin
    while (1) {
        // gets(buffer);
        scanf(" %s", &buffer);
        printf("Input: %s\n", buffer);
        cmdBuilder(buffer, 254);
    }

    return 0;
}

void initServerConfigs(char *configFile){
    FILE *file;
    printf("Server will be created using configs on: %s \n", configFile);
    file = fopen(configFile, "r");

    if(file == NULL){
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    int cmdPos = 0;
    int charCount = 0;
    char ch; 
    char readBuffer[255];
    memset(readBuffer, '\0', sizeof(readBuffer));

    while((ch = fgetc(file)) != EOF){
        // printf("Reading %c value %d \n", ch, ch);
        if(ch == '\n'){
            cmdBuilder(readBuffer, charCount);
            charCount = 0;
            memset(readBuffer, '\0', sizeof(readBuffer));
        } else {
            readBuffer[charCount] = ch;
            charCount ++;
        }
    }

    if(charCount != 0)
        cmdBuilder(readBuffer, charCount);

    fclose(file);
}

void cmdBuilder(char *readBuffer, int charCount){
    const char delim[2] = " ";
    char *token = "init";
    char *cmd;
    char *params[2];
    int paramCount = 0;

    cmd = strtok(readBuffer, delim);

    while( token != NULL ) {
      token = strtok(NULL, delim);
      params[paramCount] = token;
      paramCount ++;
    }

    if(strcmp(cmd, "add") == 0){

        host *h = createHost(params[0], params[1]); 

        printf("%s , %s", h->hostName, h->ip);

        if(addHostName(h) == -1) {
            perror("Unable to add hostname");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(cmd, "search") == 0){
        printf("Search hostname: <%s> \n", params[0]);
        //TODO
        //Create search function
    } else if (strcmp(cmd, "link") == 0){
        printf("Link ip: <%s> to port <%s>\n", params[0], params[1]);
        //TODO
        //Create link function
    } else if (strcmp(cmd, "list") == 0){
        listHostNames();
    } else {
        printf("Command: %s, not found \n", cmd);
        exit(EXIT_FAILURE);
    }

}

host *createHost(char *ip, char *hostName){
    host *h = malloc(sizeof(host));

    h->ip = malloc(strlen(ip));
    strcpy(h->ip, ip);

    h->hostName = malloc(strlen(hostName));
    strcpy(h->hostName, hostName);

    return h;
}

int addHostName(host *h) {
    printf("Add hostname: <%s> and bind to ip: <%s> \n", h->hostName, h->ip);
    hosts[hostCount] = h;
    printf("Add hostname: <%s> and bind to ip: <%s> \n", hosts[hostCount]->hostName, hosts[hostCount]->ip);
    hostCount ++;

    printf("This DNS server has now %d hosts \n", hostCount);
    for(int i = 0; i < hostCount; i++){
        printf("hostname: %s, ip %s \n", hosts[i]->hostName, hosts[i]->ip);
    }

    return 1;
}

void listHostNames(){
    printf("This DNS server has %d hosts \n", hostCount);
    // for(int i = 0; i < hostCount; i++){
    //     printf("hostname: %s, ip %s \n", &hosts[i].hostName, &hosts[i].ip);
    // }
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *port) {
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    listen(socket_desc, 3);
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    puts("Connection accepted");
    
    //Get the socket descriptor
    size_t read_size;
    char *message, client_message[2000];

    //Receive a message from client
    while ((read_size = recv(client_sock, client_message, 10, NULL)) > 0) {
        //end of string marker
        client_message[read_size] = '\0';
        printf("Server receive: %s\n", client_message);

        //Send the message back to client
        write(client_sock, client_message, strlen(client_message));

        //clear the message buffer
        memset(client_message, 0, 2000);
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    }  else if (read_size == -1) {
        perror("recv failed");
    }

    return 0;
}
