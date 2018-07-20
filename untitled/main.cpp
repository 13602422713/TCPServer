
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
using namespace std;


#include "third/TCPServer.h"



//const std::string port = "/dev/ydlidar";
const int baud = 115200;
const int intensities = 0;



TCPServer GetSensorDataServer;

void  Sensorloop()
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
        string str = GetSensorDataServer.getMessage();
        if(!tag1 && !tag2 && str != "" )
        {
//            std::cout << "Message length : " << str.length()<< std::endl;
//            GetSensorDataServer.clean();
            if(!str.compare("newmap"))
            {
                //new map come in
                GetSensorDataServer.Send("ok1");
                GetSensorDataServer.clean();
                tag1 = true;
                std::cout<<"get a newmap order"<<std::endl;
                continue;
            }
        }

        if(tag1 && !tag2 && str != "")
        {
            memcpy(&tmplength, str.data(), 4);
            GetSensorDataServer.Send("ok2");
            GetSensorDataServer.clean();
            tag2 = true;
            std::cout<<"newmap size is : "<<tmplength<<std::endl;
            continue;
        }

        if(tag1 && tag2 && str != "")
        {
            std::string outfile;
            std::stringstream tmpoutstream;
            tmpoutstream<<"./recive_picture"<<tmptag<<".png";
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
                    tmp_rec_len = GetSensorDataServer.getmsglen();
                    memcpy(tmp_recv_buf, GetSensorDataServer.getmsg(), tmp_rec_len);
                    GetSensorDataServer.clean();
                    if(1024 == tmp_rec_len)
                    {
                        tmpcircle++;
                        testout.write(tmp_recv_buf, 1024);
                        GetSensorDataServer.Send("ok3");
                    }
                    else if(tmp_rec_len < 1024 && tmp_rec_len > 0)
                    {
                        memcpy(tmp_recv_buf, GetSensorDataServer.getmsg(), tmp_rec_len);
                        int sub_tmp_rec_len;
                        while(true)
                        {
                            sub_tmp_rec_len = GetSensorDataServer.getmsglen();
                            memcpy(&tmp_recv_buf[tmp_rec_len], GetSensorDataServer.getmsg(), sub_tmp_rec_len);
                            GetSensorDataServer.clean();
                            tmp_rec_len += sub_tmp_rec_len;
                            if(1024 == tmp_rec_len)
                            {
                                tmpcircle++;
                                testout.write(tmp_recv_buf, 1024);
                                GetSensorDataServer.Send("ok3");
                                break;
                            }
                        }
                    }
                }
                else if(tmpcircle == allcircle)
                {
                    int tailen = tmplength - allcircle*1024;
                    tmp_rec_len = GetSensorDataServer.getmsglen();
                    memcpy(tmp_recv_buf, GetSensorDataServer.getmsg(), tmp_rec_len);
                    GetSensorDataServer.clean();
                    if(tailen == tmp_rec_len)
                    {
                        tmpcircle++;
                        testout.write(tmp_recv_buf, tailen);
                        GetSensorDataServer.Send("ok4");
                        tag1 = false;
                        tag2 = false;
                    }
                    else if(tmp_rec_len < tailen && tmp_rec_len > 0)
                    {
                        memcpy(tmp_recv_buf, GetSensorDataServer.getmsg(), tmp_rec_len);
                        int sub_tmp_rec_len;
                        while(true)
                        {
                            sub_tmp_rec_len = GetSensorDataServer.getmsglen();
                            memcpy(&tmp_recv_buf[tmp_rec_len], GetSensorDataServer.getmsg(), sub_tmp_rec_len);
                            GetSensorDataServer.clean();
                            tmp_rec_len += sub_tmp_rec_len;
                            if(tailen == tmp_rec_len)
                            {
                                tmpcircle++;
                                testout.write(tmp_recv_buf, tailen);
                                GetSensorDataServer.Send("ok4");
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

        //usleep(1000);
		Sleep(1);
    }
    GetSensorDataServer.detach();
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
   // pthread_t lmsg;
   // GetSensorDataServer.setup(11999);

   // if( pthread_create(&lmsg, NULL, Sensorloop, (void *)0) == 0)
	thread SensorLooopThread(Sensorloop);
	SensorLooopThread.detach();
	//if (NULL != SensorLooopThread.native_handle())
	//{
		cout << "Sensorloop was created succeed!" << endl;
		GetSensorDataServer.receive();
	//}
	
	while(1);

    return 0;
}