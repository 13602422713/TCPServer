#include "TCPServer.h"

string TCPServer::Message;
char TCPServer::msg_out[ MAXPACKETSIZE ];
int TCPServer::recv_len;

void 
TCPServer::ReceivedThread()
{
   // int n;
   // int newsockfd = (long)arg;
   // char msg[MAXPACKETSIZE];
   // while (1)
   // {
   //     n=recv(newsockfd,msg,MAXPACKETSIZE,0);
   //     if(n==0)
   //     {
			//closesocket(newsockfd);
   //         break;
   //     }
   //     msg[n]=0;
   //     recv_len = n;
   //     memcpy(msg_out, msg, recv_len);

   //     //send(newsockfd,msg,n,0);
   //     Message = string(msg);
   // }
	char msg[MAXPACKETSIZE] = { 0 };
	while (1)
	{

		int n = recv(sClientSockfd, msg, MAXPACKETSIZE, 0);
		//接收长度为0
		if (n == 0)
		{
			closesocket(sClientSockfd);
			break;
		}

		msg[n] = 0;//what the fuck!?
		recv_len = n;
		memcpy(msg_out, msg, recv_len);

		//send(sClientSockfd,msg,n,0);
		Message = string(msg);
	}
}

char*
TCPServer::getmsg()
{
    return msg_out;
}

int
TCPServer::getmsglen()
{
    return recv_len;
}

void
TCPServer::setup(int port)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    listen(sockfd, 5);
}

string
TCPServer::receive()
{
    string str;
    while (1)
    {
		_Inout_opt_ int FAR  sosize = sizeof(clientAddress);
        //newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &sosize);
		sClientSockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &sosize);
        //str = itoa(clientAddress.sin_addr);
		stringstream ss;
		ss << (uint32_t)clientAddress.sin_addr.s_addr;
		str = ss.str();		//获取IP地址

		//创建接收线程
		serverThread = thread(&TCPServer::ReceivedThread , this);
		serverThread.detach();
    }
    return str;
}

string
TCPServer::getMessage()
{
    return Message;
}

void
TCPServer::Send(string msg)
{
    send(newsockfd, msg.c_str(), msg.length(), 0);
}

void
TCPServer::clean()
{
    Message = "";
    memset(msg, 0, MAXPACKETSIZE);
    memset(msg_out, 0, MAXPACKETSIZE);
    recv_len = 0;
}

void
TCPServer::detach()
{
	closesocket(sockfd);
	closesocket(newsockfd);
} 
