#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define LENGTH 1024

char hostname[50];
char passcode[8];
static volatile int leave = 1;
int sock;

void signalHandler(int e) {
    leave = 0;
}

void sendMessage() {

	char message[LENGTH];
	int space = 0;

	while(1) {
		space = 0;
		fgets(message, LENGTH, stdin);
		if (strlen(message) > LENGTH) {
			printf("Message too long\n");
		} else {
			for (int i = 0; i < LENGTH; i++) {
				if (message[i] == '\n') {
					message[i] = '\0';
					break;
				} else if (message[i] == ' ') {
					space = 1;
				}
			}
			if (message[0] == ':' && space == 0) {
				char *special = strtok(message, " ");
				if (!strcmp(special, ":Exit")) {
					break;
				}
			}
			if (strlen(message) > 0) {
				send(sock, message, strlen(message), 0);
			}
		}
		memset(message, 0, LENGTH);
	}
	leave = 0;
}

void receiveMessage() {
	char message[LENGTH];
	while (1) {
		recv(sock, message, LENGTH, 0);
		if (strlen(message) > LENGTH) {
			printf("Message too long\n");
		} else {
			printf("%s", message);
		}
		memset(message, 0, LENGTH);
	}
	leave = 0;
}

int main(int argc, char **argv){

	signal(SIGINT, signalHandler);

	if(argc < 2) {
		printf("Need port number\n");
		return 1;
	}

	printf("Please enter your username: ");
	fgets(hostname, 50, stdin);

	for (int i = 0; i < LENGTH; i++) {
		if (hostname[i] == '\n') {
			hostname[i] = '\0';
			break;
		}
	}

	if (strlen(hostname) > 50) {
		printf("Username longer than 50 characters.\n");
		return 1;
	}

	printf("Please enter the passcode: ");
	fgets(passcode, 8, stdin);
	if (strcmp("secretp", passcode) != 0){
		printf("Passcode Incorrect.\n");
		return 1;
	}

	pthread_t sendThread;
	pthread_t receiveThread;
	int port = atoi(argv[1]);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0) {
		printf("Cannot connect\n");
		return 1;
	}

	send(sock, hostname, 50, 0);

	printf("~~~ You have entered CS3251 Chat Room ~~~\n");

	if(pthread_create(&sendThread, NULL, (void *) sendMessage, NULL) != 0){
		printf("thread error\n");
		return 1;
	}

	if(pthread_create(&receiveThread, NULL, (void *) receiveMessage, NULL) != 0){
		printf("thread error\n");
		return 1;
	}

	while (leave){
		//idle
	}

	printf("\nYou left the chat room\n");
	close(sock);

	return 0;
}
