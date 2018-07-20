#include "TCPServer.h"

string TCPServer::Message;
char TCPServer::msg_out[ MAXPACKETSIZE ];
int TCPServer::recv_len;

void *
TCPServer::Task(void *arg)
{
    int n;
    int newsockfd = (long) arg;
    char msg[MAXPACKETSIZE];
	serverThread.detach();
    while (1)
    {
        n=recv(newsockfd,msg,MAXPACKETSIZE,0);
        if(n==0)
        {
			closesocket(newsockfd);
            break;
        }
        msg[n]=0;
        recv_len = n;
        memcpy(msg_out, msg, recv_len);

        //send(newsockfd,msg,n,0);
        Message = string(msg);
    }
    return 0;
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
        newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &sosize);
        str = inet_ntoa(clientAddress.sin_addr);
		serverThread = thread(Task);
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
