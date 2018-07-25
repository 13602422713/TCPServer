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
	TCPServer(int port);
	TCPServer::TCPServer();
	TCPServer::~TCPServer();

	char* getmsg();
	int getmsglen();

	void ReceivedThread();

	bool setup(int port);
	string waitingConnect();
	void beginRecived();
	string receive();
	string getMessage();
	void Send(string msg);
	void detach();
	void clean();

public:
	static string Message;
	static char msg_out[ MAXPACKETSIZE ];
	static int recv_len;
private:
	SOCKET c_lSockfd, c_lClientSockfd;
	struct sockaddr_in c_stServerAddress;
	struct sockaddr_in c_stClientAddress;
	thread c_cServerThread;
};

#endif
