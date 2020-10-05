/*
    Compile
    gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> //for threading

#define NOT_FOUND "404 Not Found"
#define MAX_HOSTS 100
#define MAX_DNSNODES 5
#define MAXLINE 1024
#define PORT 5535 

typedef struct host{
    char *hostName;
    char *ip;
} host;

typedef struct dnsNode {
    char *ip;
    char *port;
} dnsNode;

int hostCount = 0;
int dnsNodeCount = 0;
host *hosts[MAX_HOSTS];
dnsNode *dnsNodes[5];

void *connection_handler(void *);
void initServerConfigs(char *configFile);
char *cmdBuilder(char *readBuffer, int charCount);
int addHostName(host *h);
void listHosts();
host *createHost(char *hostName, char *ip);
char *localSearchHost(char *hostName);
char *externalSearchHost(dnsNode *node, char *hostName);
void sendMessage(char *port, char *message);
char *rcvMessage(void *port, bool sholdConvertPort);

dnsNode *createDnsNode(char *ip, char *port);

int main(int argc, char *argv[]) {

    pthread_t thread_id;
    char buffer[254];

    if (argc > 2) {
        initServerConfigs(argv[2]);
    }

    int port = atoi(argv[1]);
    
    // create thread for socket 
    if (pthread_create(&thread_id, NULL, connection_handler, (void*) port) < 0) {
        perror("could not create thread");
        return 1;
    }

    // continue listening to stdin
    while (1) {
        gets(buffer);
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

char* cmdBuilder(char *readBuffer, int charCount){
    const char delim[2] = " ";
    char *token = "init";
    char *cmd;
    char *params[2];
    int paramCount = 0;
    char *response;

    cmd = strtok(readBuffer, delim);

    while( token != NULL ) {
      token = strtok(NULL, delim);
      params[paramCount] = token;
      paramCount ++;
    }

    if(strcmp(cmd, "add") == 0){

        host *h = createHost(params[0], params[1]); 

        if(addHostName(h) == -1) {
            perror("Unable to add hostname");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(cmd, "search") == 0){
        char *searchResult;
        searchResult = localSearchHost(params[0]);
        if(strcmp(searchResult, NOT_FOUND) == 0) {
            for(int i = 0; i < dnsNodeCount; i++){
                searchResult = externalSearchHost(dnsNodes[i], params[0]);
                if(strcmp(searchResult, NOT_FOUND) != 0){
                    printf("Hostname: %s is registered on ip: %s \n", params[0], searchResult);
                    return(searchResult);
                }
            }
        } else {
            printf("Hostname: %s is registered on ip: %s \n", params[0], searchResult);
            return(searchResult);
        }
    } else if (strcmp(cmd, "link") == 0){

        dnsNode *n = createDnsNode(params[0], params[1]);
        
        if(addDnsNode(n) == -1) {
            perror("Unable to link node");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(cmd, "list") == 0){
        listHostNames();
    } else {
        printf("Command: %s, not found \n", cmd);
        exit(EXIT_FAILURE);
    }

}

char *localSearchHost(char *hostName){
    printf("Searching on local server for hostname: <%s> \n", hostName);
    for(int i = 0; i < hostCount; i++){
        // printf("Comparing %d: ", i);
        // printf("host: %s ,len: %d , lastchar: %c -> ", hostName, strlen(hostName), hostName[strlen(hostName) - 1]);
        // printf("host: %s ,len: %d , lastchar: %c \n", hosts[i]->hostName, strlen(hosts[i]->hostName), hosts[i]->hostName[strlen(hosts[i]->hostName) - 1]);
        if(strcmp(hosts[i]->hostName, hostName) == 0){
            return(hosts[i]->ip);
        }
    }
    return(NOT_FOUND);
}

char *externalSearchHost(dnsNode *node, char *hostName){
    printf("Sending search command to on external server: %s:%s \n", node->ip, node->port);
    char *message[255];
    strcpy(message, "search ");
    strcat(message, hostName);
    sendMessage(node->port, message);
    char *searchResult = rcvMessage(node->port, false);
    return(searchResult);
}

host *createHost(char *hostName, char *ip){
    host *h = malloc(sizeof(host));

    h->ip = malloc(strlen(ip));
    strcpy(h->ip, ip);

    h->hostName = malloc(strlen(hostName));
    strcpy(h->hostName, hostName);

    return h;
}

int addHostName(host *h) {
    hosts[hostCount] = h;
    printf("Add hostname: <%s> and bind to ip: <%s> \n", hosts[hostCount]->hostName, hosts[hostCount]->ip);
    hostCount ++;

    return 1;
}

void listHostNames(){
    printf("This DNS server has %d hosts \n", hostCount);
    for(int i = 0; i < hostCount; i++){
        printf("hostname: %s, ip %s \n", hosts[i]->hostName, hosts[i]->ip);
    }
}

dnsNode *createDnsNode(char *ip, char *port){
    dnsNode *n = malloc(sizeof(dnsNode));

    n->ip = malloc(strlen(ip));
    strcpy(n->ip, ip);

    n->port = malloc(strlen(port));
    strcpy(n->port, port);

    return n;
}

int addDnsNode(dnsNode *n) {
    dnsNodes[dnsNodeCount] = n;
    printf("Add node to dns server: %s:%s \n", dnsNodes[dnsNodeCount]->ip, dnsNodes[dnsNodeCount]->port);
    dnsNodeCount ++;

    return 1;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *port) {
    int sockfd; 
	char buffer[MAXLINE]; 
	char *hello = "Hello from server"; 
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(port);
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
    printf("Bind succeed on port:%d, htons converted = %d \n", port, servaddr.sin_port);

	int len, n; 

	len = sizeof(cliaddr);

	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 
    
    char *message[strlen(buffer) - 1]; 
    strncpy(message, buffer, strlen(buffer));

	 
	printf("Received : %s \n", message);
    char *response;
    response = cmdBuilder(message, strlen(message));

    if(response != NULL)
        sendMessage(&cliaddr.sin_port, response);

	// sendto(sockfd, (const char *)hello, strlen(hello), 
	// 	MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
	// 		len); 
	printf("Message sent: %s\n", response); 
	return 0;
}

void sendMessage(char *port, char *message){
    int sockfd; 
	struct sockaddr_in	 servaddr; 

    int p = atoi(port);

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(p); 
	servaddr.sin_addr.s_addr = INADDR_ANY; 

    printf("port:%d, htons converted = %d \n", port, servaddr.sin_port);
	
	int n, len; 
	
	sendto(sockfd, (const char *)message, strlen(message), 
		MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr)); 
	printf("Message sent: %s\n", message); 
}

char *rcvMessage(void *port, bool sholdConvertPort){
    int sockfd; 
	char buffer[MAXLINE]; 
	struct sockaddr_in	 servaddr; 
    int p;

    if(sholdConvertPort)
        p = atoi(port);

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET;
    if(sholdConvertPort)
	    servaddr.sin_port = htons(p); 
    else
       servaddr.sin_port = p; 
	servaddr.sin_addr.s_addr = INADDR_ANY; 

    printf("port:%d, htons converted = %d \n", port, servaddr.sin_port);
	
	int n, len; 
	
	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &servaddr, 
				&len); 
	
    return(buffer);
}