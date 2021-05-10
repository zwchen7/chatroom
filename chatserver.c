#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX 80
#define BUFFER 1024

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct client{
	char hostname[50];
	struct sockaddr_in address;
	int sock;
};

struct client *client[MAX];

void addClient(struct client *c){
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MAX; i++){
		if(!client[i]){
			client[i] = c;
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
}

void removeClient(struct client *c, char *hostname){
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MAX; i++){
		if(client[i]){
			if(strcmp(hostname, client[i]->hostname) == 0){
				client[i] = NULL;
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

void sendToAll(char *message, char *hostname){
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MAX; i++){
		if(client[i]) {
			if(strcmp(hostname, client[i]->hostname) != 0){
				write(client[i] -> sock, message, strlen(message));
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

void sendToSelf(char *message, char *hostname){
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MAX; i++){
		if(client[i]) {
			if(strcmp(hostname, client[i]->hostname) == 0){
				write(client[i] -> sock, message, strlen(message));
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

void* network(void * ClientDetail){
	struct client* clientDetail = (struct client*) ClientDetail;
	FILE *f = fopen("chatlog.txt", "w");
	char hostname[50];
	char output[BUFFER];
	char message[BUFFER];
	int space = 0;
	int all = 0;
	recv(clientDetail->sock, hostname, 50, 0);
	strcpy(clientDetail -> hostname, hostname);
	sprintf(output, "%s joined!\n", clientDetail->hostname);
	printf("%s", output);
	fprintf(f, "%s", output);
	sendToAll(output, clientDetail->hostname);
	memset(output, 0, BUFFER);
	memset(message, 0, BUFFER);

	while(1){
		space = 0;
		int read = recv(clientDetail->sock, message, BUFFER, 0);
		if (read){
			all = 0;
			for (int i = 0; i < BUFFER; i++) {
				if (message[i] == '\n') {
					message[i] = '\0';
					break;
				} else if (message[i] == ' ') {
					space = 1;
				}
			}
			if (message[0] == ':' && space == 0) {
				char *special = strtok(message, " ");
				if (!strcmp(special, ":)")) {
					all = 1;
					strcpy(message, "[feeling happy]");
				} else if (!strcmp(special, ":(")) {
					all = 1;
					strcpy(message, "[feeling sad]");
				} else if (!strcmp(special, ":mytime")) {
					time_t mytime = time(NULL);
					char* time = ctime(&mytime);
					char* token;
					token = strtok(time, " ");
					token = strtok(NULL, " ");
					token = strtok(NULL, " ");
					token = strtok(NULL, " ");
					all = 1;
					strcpy(message, token);
				} else if (!strcmp(special, ":+1hr")) {
					time_t mytime = time(NULL);
					char* time = ctime(&mytime);
					char* token;
					token = strtok(time, " ");
					token = strtok(NULL, " ");
					token = strtok(NULL, " ");
					token = strtok(NULL, " ");
					int temp = token[1] - '0';
					int temp2 = token[0] - '0';
					if (temp == 3 && temp2 == 2) {
						temp = 0;
						temp2 = 0;
						char convert = temp + '0';
						char convert2 = temp2 + '0';
						token[0] = convert2;
						token[1] = convert;
					} else if (temp==9) {
						temp = 0;
						temp2++;
						char convert = temp + '0';
						char convert2 = temp2 + '0';
						token[0] = convert2;
						token[1] = convert;
					} else {
						temp++;
						char convert = temp + '0';

						token[1] = convert;
					}
					all = 1;
					strcpy(message, token);
				}
			}
			char output[BUFFER];
			if (strlen(message) > 0) {
				sprintf(output, "%s: %s\n", hostname, message);
			}
			sendToAll(output, clientDetail->hostname);
			if (all == 1) {
				sendToSelf(output, clientDetail->hostname);
			}
			for (int i = 0; i < BUFFER; i++) {
				if (output[i] == '\n') {
					output[i] = '\0';
					break;
				}
			}
			printf("%s\n", output);
			fprintf(f, "%s\n", output);
			memset(message, 0, BUFFER);
			memset(output, 0, BUFFER);
		} else {
			sprintf(output, "%s has left\n", clientDetail -> hostname);
			printf("%s", output);
			fprintf(f, "%s", output);
			removeClient(clientDetail, clientDetail -> hostname);
			sendToAll(output, clientDetail->hostname);
			break;
		}
		memset(output, 0, BUFFER);
	}
	fclose(f);
	return NULL;
}


int main(int argc, char **argv){

	if(argc < 2) {
		printf("Need port number\n");
		return 1;
	}

	pthread_t thread;
	int port = atoi(argv[1]);
	int clientSocket = 0;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		perror("Binding error");
		return 1;
	}

	if (listen(serverSocket, 100) < 0) {
		perror("Listening error");
		return 1;
	}

	printf("~~~ You have entered CS3251 Chat Room ~~~\n");

	while(1){
		socklen_t clen = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clen);
		struct client *c = (struct client*) malloc(sizeof(struct client));
		c->address = clientAddress;
		c->sock = clientSocket;
		addClient(c);
		pthread_create(&thread, NULL, &network, (void*)c);
	}

	return 0;
}
