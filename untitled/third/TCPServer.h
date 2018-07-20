#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <sys/types.h> 
//#include <sys/socket.h>
/*#include <WINSOCK2.H> */  
#include <Winsock.h>
//#include <netinet/in.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <pthread.h>
#include <thread>
#include <sstream>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define MAXPACKETSIZE 1024*10

class TCPServer
{
public:
	int sockfd, newsockfd, sClientSockfd,n, pid;
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	thread serverThread;
	char msg[ MAXPACKETSIZE ];
	static string Message;
	static char msg_out[ MAXPACKETSIZE ];
	static int recv_len;
public:
	void setup(int port);
	string receive();
	string getMessage();
	char* getmsg();
	int getmsglen();
	void Send(string msg);
	void detach();
	void clean();

public:
	void ReceivedThread();
};

#endif
