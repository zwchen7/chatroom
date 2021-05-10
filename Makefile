all: chatserver chatclient

chatserver: chatserver.c
	gcc chatserver.c -o chatserver -lpthread

chatclient: chatclient.c
	gcc chatclient.c -o chatclient -lpthread
