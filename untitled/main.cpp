#include <iostream>
#include <sstream>
#include <signal.h>
#include <memory>
#include <windows.h>
#include <thread>
#include <vector>
#include <map>
#include <fstream> 
#include <string> 
#include "third/TCPServer.h"

using namespace std;


TCPServer cPictureServer;



void handleThread()
{
	bool tag1 = false;
	bool tag2 = false;
	bool tag3 = false;
	int tmplength = 0;
	int tmpcircle = 0;
	int tmptag = 0;
	std::cout<<"start to receive png : "<<std::endl;
	while(1)
	{
		string str = cPictureServer.getMessage();
		if(!tag1 && !tag2 && str != "" )
		{
//            std::cout << "Message length : " << str.length()<< std::endl;
//            cPictureServer.clean();
			if(!str.compare("newmap"))
			{
				cPictureServer.Send("ok1");
				cPictureServer.clean();
				tag1 = true;
				std::cout<<"get a newmap order"<<std::endl;
				continue;
			}
		}

		if(tag1 && !tag2 && str != "")
		{
			memcpy(&tmplength, str.data(), 4);
			cPictureServer.Send("ok2");
			cPictureServer.clean();
			tag2 = true;
			std::cout<<"newmap size is : "<<tmplength<<std::endl;
			continue;
		}

		if(tag1 && tag2 && str != "")
		{
			std::string outfile;
			std::stringstream tmpoutstream;
			tmpoutstream<<tmptag<<".png";
			outfile = tmpoutstream.str();
			std::cout<<"receive file : "<<outfile<<std::endl;
			tmptag++;
			std::ofstream testout(outfile.data(),ios::binary|ios::out);

			int allcircle = tmplength / 1024;
			while(true)
			{
				char * tmp_recv_buf = new char[1024];
				int tmp_rec_len;
				if(tmpcircle < allcircle)
				{
					tmp_rec_len = cPictureServer.getmsglen();
					memcpy(tmp_recv_buf, cPictureServer.getmsg(), tmp_rec_len);
					cPictureServer.clean();
					if(1024 == tmp_rec_len)//接收到1024byte的帧
					{
						tmpcircle++;
						testout.write(tmp_recv_buf, 1024);
						cPictureServer.Send("ok3");
						cout<<"tmpcircle:"<<tmpcircle<<endl;
					}
					else if(tmp_rec_len < 1024 && tmp_rec_len > 0)//接收到0-1024byte的帧
					{
						memcpy(tmp_recv_buf, cPictureServer.getmsg(), tmp_rec_len);
						int sub_tmp_rec_len;
						while(true)
						{
							sub_tmp_rec_len = cPictureServer.getmsglen();
							memcpy(&tmp_recv_buf[tmp_rec_len], cPictureServer.getmsg(), sub_tmp_rec_len);
							cPictureServer.clean();
							tmp_rec_len += sub_tmp_rec_len;
							if(1024 == tmp_rec_len)
							{
								tmpcircle++;
								testout.write(tmp_recv_buf, 1024);
								cPictureServer.Send("ok3");
								cout<<"tmpcircle:"<<tmpcircle<<endl;
								break;
							}
						}
					}
					else if(tmp_rec_len>1024)
					{
						
					}
				}
				else if(tmpcircle == allcircle)//last frame
				{
					int tailen = tmplength - allcircle*1024;
					tmp_rec_len = cPictureServer.getmsglen();
					memcpy(tmp_recv_buf, cPictureServer.getmsg(), tmp_rec_len);
					cPictureServer.clean();
					if(tailen == tmp_rec_len)
					{
						tmpcircle++;
						testout.write(tmp_recv_buf, tailen);
						cPictureServer.Send("ok4");
						tag1 = false;
						tag2 = false;
					}
					else if(tmp_rec_len < tailen && tmp_rec_len > 0)
					{
						memcpy(tmp_recv_buf, cPictureServer.getmsg(), tmp_rec_len);
						int sub_tmp_rec_len;
						while(true)
						{
							sub_tmp_rec_len = cPictureServer.getmsglen();
							memcpy(&tmp_recv_buf[tmp_rec_len], cPictureServer.getmsg(), sub_tmp_rec_len);
							cPictureServer.clean();
							tmp_rec_len += sub_tmp_rec_len;
							if(tailen == tmp_rec_len)
							{
								tmpcircle++;
								testout.write(tmp_recv_buf, tailen);
								cPictureServer.Send("ok4");
								tag1 = false;
								tag2 = false;
								break;
							}
						}
					}
				}
				else
				{
					break;
				}
			}
			testout.close();
		}

		Sleep(5);
	}
	cPictureServer.detach();
}

int main(int argc, char *argv[])
{
	//    std::thread dealydliar(GetYdlidarData);
	//    dealydliar.join();
	/*
	std::map<int, int> trymap;
	int &trymap_value = trymap[1];
	trymap_value = 100;
	std::cout<<trymap[1]<<std::endl;
	std::cout<<trymap.size()<<std::endl;

	for(int i = 0; i < 10; i++)
	{
		std::string tmpstr;
		std::stringstream tmpstream;
		tmpstream<<i;
		tmpstr = tmpstream.str();
		tmpstr += ".png";
		std::cout<<tmpstr.data()<<std::endl;
	}
	*/

	//Setup the picture server.
	if( !cPictureServer.setup(11999) )
	{
		cout<<"PictureServer create fail"<<endl;
		system("pause");
		return 0;
	}

	//Setup handle thread
	thread cHandleThread(handleThread);
	cHandleThread.detach();
	cout << "handleThread was created succeed!" << endl;

	// if( pthread_create(&lmsg, NULL, handleThread, (void *)0) == 0)
	//if (NULL != SensorLooopThread.native_handle())
	//{
	//	cout << "handleThread was created succeed!" << endl;
	//	cPictureServer.receive();
	//}

	//Waiiting request comming to receive datas
	cPictureServer.waitingConnect();
	cPictureServer.beginRecived();

	while(1);

	return 0;
}