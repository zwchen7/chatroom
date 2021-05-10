# chatroom
Chatroom application using multi-threading and sockets to allow multiple clients to connect to one server

To compile the files simply run make. Another way to compile is running gcc chatserver.c -o chatserver -lpthread, and gcc chatclient.c -o chatclient -lpthread.

Files tested in Ubuntu 18.04.5

To start the server run ./chatserver <port>. For example ./chatserver 5001
To start the client run ./chatclient <port>, for example ./chatclient 5001, the port number needs to be opened by a server first. You will be prompted to enter a username and passcode afterwards, the passcode is secretp.

Server log will be outputted to chatlog.txt
