#include "TCPServer.h"

string TCPServer::Message;
char TCPServer::msg_out[ MAXPACKETSIZE ];
int TCPServer::recv_len;

TCPServer::TCPServer(int port)
{
	setup(port );		
}
TCPServer::TCPServer()
{

}
TCPServer::~TCPServer()
{
	WSACleanup();
	detach();	
}


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
	memset(msg, 0, MAXPACKETSIZE);

	while (1)
	{
		int n = recv(c_lClientSockfd, msg, MAXPACKETSIZE, 0);
		if( -1 == n)
		{
			//empty
		}
		else
		{
			if (n == 0)	//接收长度为0
			{
				closesocket(c_lClientSockfd);
				break;
			}

			//msg[n] = 0;//what the fuck is it !?
			recv_len = n;
			memcpy(msg_out, msg, recv_len);

			//send(c_lClientSockfd,msg,n,0);
			Message = string(msg);
			memset(msg, 0, recv_len);
		}
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


bool 
TCPServer::setup(int port)
{
	WORD sockVersion = MAKEWORD(1,1); 
	WSADATA wsaData;

	//WSA加载
    if(WSAStartup(sockVersion, &wsaData)!=0)  
    {  
		cout<<" WAS err"<< endl;
        return false;  
    } 

	//socket
	c_lSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( INVALID_SOCKET == c_lSockfd )  
    {  
        cout<<"socket error !"<<endl;  
        return false;  
    }

	//设置地址和端口
	memset(&c_stServerAddress, 0, sizeof(c_stServerAddress));
	c_stServerAddress.sin_family = AF_INET;
	c_stServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);    //有IP
	c_stServerAddress.sin_port = htons(port);			   //设置监听端口

	//bind
	if( SOCKET_ERROR == ::bind(c_lSockfd,(SOCKADDR *) &c_stServerAddress,sizeof(SOCKADDR)) )
	{
		cout<<"bind failed"<<endl;
		return false;
	}

	//listen
	if( SOCKET_ERROR == listen(c_lSockfd, 5))
	{
		cout<<"listen err"<<endl;
		return false; 
	}
	return true;
}

string
TCPServer::waitingConnect()
{
	string str;
	//等待连接
    while (INVALID_SOCKET == c_lClientSockfd)
    {
		_Inout_opt_ int FAR  sosize = sizeof(c_stClientAddress);
		c_lClientSockfd = accept(c_lSockfd, (struct sockaddr *) &c_stClientAddress, &sosize);
		Sleep(10);
	}
	cout<<"Connection is comming"<<endl;

	//获取IP地址（string format）
	//str = itoa(c_stClientAddress.sin_addr);
	stringstream ss;
	ss << (uint32_t)c_stClientAddress.sin_addr.s_addr;
	str = ss.str();

	return str;
}

void
TCPServer::beginRecived()
{
	//创建接收线程
	c_cServerThread = thread(&TCPServer::ReceivedThread , this);
	c_cServerThread.detach();	
}

string
TCPServer::receive()
{
    string str;
    while (1)
    {
		_Inout_opt_ int FAR  sosize = sizeof(c_stClientAddress);
		c_lClientSockfd = accept(c_lSockfd, (struct sockaddr *) &c_stClientAddress, &sosize);
		if( INVALID_SOCKET == c_lClientSockfd )  
		{  
			Sleep(10);
			continue;  
		}
		cout<<"Connection is comming"<<endl;

		//获取IP地址（string format)
		//str = itoa(c_stClientAddress.sin_addr);
		stringstream ss;
		ss << (uint32_t)c_stClientAddress.sin_addr.s_addr;
		str = ss.str();		

		//创建接收线程
		c_cServerThread = thread(&TCPServer::ReceivedThread , this);
		c_cServerThread.detach();

		break;
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
    send(c_lClientSockfd, msg.c_str(), msg.length(), 0);
}

void
TCPServer::clean()
{
    Message = "";
    memset(msg_out, 0, MAXPACKETSIZE);
    recv_len = 0;
}

void
TCPServer::detach()
{
	closesocket(c_lSockfd);
	closesocket(c_lClientSockfd);
} 
