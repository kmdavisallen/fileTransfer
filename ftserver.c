/**********************************************************************************
**Author: Kevin Allen
**Program: ftserver
**Description:  server side of a basic file transfer program, much of the socket setup was adapted
	from Beej's guide to network programming : https://beej.us/guide/bgnet/, many of the functions are 
	adapted/copied from CS372 program 1
**CS 372 Program 2
**Last modified 11/20/18
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

/*
function to send all bytes in the buffer
pre-conditions: takes socket existing file descriptor, a buffer containing the message, and the size of the buffer
post-conditions: all bytes in tthe buffer trasnfered through socket, returns number of bytes sent
*/
int sendAll(int socket, char *mes, int len) {		//fucntion taken from Beej's guide https://beej.us/guide/bgnet/html/multi/advanced.html
	int totalSent = 0;
	int bytesRemain = len;
	int sent;

	while (totalSent < len) {
		sent = send(socket, mes + totalSent, bytesRemain, 0);
		if (sent == -1) {
			return -1;
		}
		totalSent += sent;
		bytesRemain -= sent;
	}
	return totalSent;
}


/*function to recieve messages
pre-conditions: takes existing socket file descriptor, a existiong buffer for the message and the size of the buffer
post-conditions: buffer will contain message sent through the socket, returns number of bytes recueved
*/
int recieveMess(int socket, char *buffer, int size) {	
	int bytes;
	memset(buffer, '\0', size);				//clear out buffer
	if ((bytes = recv(socket, buffer, size, 0)) == 0) {		//recieve message
		printf("recieve failed\n");
		return -1;
	}
	return bytes;
}

/*function to connect to a TCP socket
pre-conditions: takes a valid host name, port number, and a pointer to the struct addrinfo to hold the connection infromation
post-conditions: returns a socket file descriptor, socket setup was adapted from Beej's guide to network programming : https://beej.us/guide/bgnet/
*/
int connectTo(char* host, char* port, struct addrinfo *gc) {
	struct addrinfo hints;
	struct addrinfo *res;
	int sockFD;

	//setup hints struct
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(host, port, &hints, &res) != 0) {
		fprintf(stderr, "getaddrinfo failed\n");
		return 1;
	}

	for (gc = res; gc != NULL; gc = gc->ai_next) {
		if ((sockFD = socket(gc->ai_family, gc->ai_socktype, gc->ai_protocol)) == -1) {
			fprintf(stderr, "socket error");
			continue;
		}
		if (connect(sockFD, gc->ai_addr, gc->ai_addrlen) == -1) {
			fprintf(stderr, "connection error");
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	return sockFD;
}

/*function to create server side TCP socket
pre-conditions: takes a port number and a pointer to struct addrinfo
post-conditions: returns socket file descriptor, socket setup adapted from Beej's guide to network programing https://beej.us/guide/bgnet/
*/
int createSocket(char* port, struct addrinfo *gc) {
	struct addrinfo hints;
	struct addrinfo *res;
	int sockFD;
	int yes = 1;

	//setup hints struct
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &res) != 0) {
		fprintf(stderr, "getaddrinfo failed\n");
		return 1;
	}

	for (gc = res; gc != NULL; gc = gc->ai_next) {
		if ((sockFD = socket(gc->ai_family, gc->ai_socktype, gc->ai_protocol)) == -1) {
			fprintf(stderr, "socket error\n");
			continue;
		}
		if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			fprintf(stderr, "socket options error\n");
			return -1;
		}
		if (bind(sockFD, gc->ai_addr, gc->ai_addrlen) == -1) {
			fprintf(stderr, "connection error\n");
			close(sockFD);
			continue;
		}
		break;
	}
	freeaddrinfo(res);

	if (gc == NULL) {
		fprintf(stderr, "server failed to bind\n");
	}
	return sockFD;
}

/*
function to process request from client
pre-conditions: socket is setup, cmd is a valid command
post-conditons: sends data through the socket returns 1 on sucess, -1 on failure
*/
void handleRequest(int dataSocket, int controlSocket, char *cmd, char *fileName, char *port) {
	char buffer[1024];
	memset(buffer, '\0', sizeof(buffer));				//clear out buffer
	if (strcmp(cmd, "-l") == 0) {
		char dirBuf[1024];
		memset(dirBuf, '\0', sizeof(dirBuf));
		fprintf(stdout, "request for directory listing on port %s\n", port);
		struct dirent *entry;				//directory reading function adapted from https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
		DIR *directory = opendir(".");
		while ((entry = readdir(directory)) != NULL) {
			sprintf(dirBuf, "%s", entry->d_name);
			sendAll(dataSocket, dirBuf, sizeof(dirBuf));
			memset(dirBuf, '\0', sizeof dirBuf);
		}
		closedir(directory);

		close(dataSocket);
	}
	else if (strcmp(cmd, "-g") == 0) {
		FILE *file;
		file = fopen(fileName, "r");		//open requested file
		if (!file) {
			sprintf(buffer, "file not found");
			sendAll(controlSocket, buffer, 14);
			close(dataSocket);
		}
		else {
			sprintf(buffer, "recieving file %s", fileName);
			sendAll(controlSocket, buffer, sizeof(buffer));

			//send whole file code adapted from https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
			//get size of file
			fseek(file, 0, SEEK_END);
			int fileSize = ftell(file);
			fseek(file, 0, SEEK_SET);	//reset the pointer to the beginning
			
			printf("sending file %s\n", fileName);
			char* fileBuf = malloc(fileSize + 1);
			fread(fileBuf, fileSize, 1, file);
			sendAll(dataSocket, fileBuf, fileSize);
			
			fclose(file);
			free(fileBuf);
			close(dataSocket);
		}
	}
}
//main program driver
int main(int argc, char *argv[]) {
	struct addrinfo *gc;
	struct sockaddr_storage clientControl;
	struct addrinfo *clientData;
	socklen_t sin_size;
	int controlSock;
	int controlCon;

	char buffer[1024];
	char clientIP[INET6_ADDRSTRLEN];

	//convert port number to an int and validate it
	int numPort = atoi(argv[1]);
	if (numPort <= 49152) {
		printf("The port number you entered is reserved, please chose a port > 49152 and < 65535 \n");
		return 1;
	}
	if (numPort > 65535) {
		printf("The port number you entered is too high, please chose a port > 49152 and < 65535 \n"); 
		return 1;
	}

	//create socket
	controlSock = createSocket(argv[1], gc);
	// listen on socket and accept connection
	listen(controlSock, 1);
	fprintf(stdout, "Server listening on port %s\n", argv[1]);
	while (1) {
						
		sin_size = sizeof(clientControl);
		controlCon = accept(controlSock, (struct sockaddr *)&clientControl, &sin_size);
		
		if (controlCon != -1) {
			fprintf(stdout, "Connection established\n");
		}
		
		//get commands from the client and parse data
		recieveMess(controlCon, buffer, sizeof(buffer));		
		char *fileNm = NULL;
		char *port = NULL;
		char *command = strtok(buffer, " ");	//get the command
		if (strcmp(command, "-g") == 0) {	
			fileNm = strtok(NULL, " ");	//get file name
		}
		port = strtok(NULL, " \n");		//get port number
		
		//get client address, taken from Beej's guide to network programming as refrenced abouve
		inet_ntop(clientControl.ss_family, &((struct sockaddr_in *)&clientControl)->sin_addr, clientIP, sizeof(clientIP));

		//connect to port specified by client
		int dataPort = connectTo(clientIP, port, clientData);

		//send info over data connection
		handleRequest(dataPort, controlCon, command, fileNm, port);
	
	}
	return 0;
}