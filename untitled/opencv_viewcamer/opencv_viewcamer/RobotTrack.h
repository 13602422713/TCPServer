#ifndef __ROBOTTRACK_H
#define __ROBOTTRACK_H

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>


using namespace cv;
using namespace std;


class RobotTrack
{
public:
	RobotTrack(VideoCapture &cap);//ʹ�ô˹��캯��ʱ�������и��µ�ʱ������ߵ���setTarget()�趨Ŀ��
	RobotTrack(VideoCapture &cap, Rect target);
	bool setTarget(Rect target);
	bool setOutput(VideoWriter &writer);
	~RobotTrack();
	bool updateRobotPlace(Vec3f &robotCycle);
	bool showTrack();
	Mat getLast();

private:
	//	void RobotTrack::mixRouteAndLine();
	int mixRouteAndLine();
	bool selectoverlappingCycle(vector<Vec3f> &vec, RotatedRect m_cTrackBox);
	bool selectRadiusBestCycle(vector<Vec3f> &vec, Vec3f &cycle);
	bool isTackWindowNeedReset();
	bool resetTrackWindow();
	bool getVedioFrame();
	bool isTrackSrcDidSet();
	bool ouput();
private:
	/*��Ƶ���*/
	VideoWriter *m_cWriter;
	/**/
	vector<Vec3f> m_vCircles;
	/*camshift*/
	VideoCapture m_pVedio;
	Rect m_cSrcTarget;//Դ����
	Mat m_cFrame;	//��ʱ��Ƶ֡
	Mat m_cHsv;		//HSV��ʽ
	Mat m_cMask;    //���루��ֵͼ��
	Mat m_cHue;		//��HSCV�з������ɫ��ͼ
	Mat m_cHist;	//ɫ��ֲ�ͼ
	RotatedRect m_cTrackBox;//CamShift׷�ٽ��
	bool m_bFlagOfTrackWindowNeedToSet;//�Ƿ���Ҫ���²ɴ���
	Rect m_cTrackWindow;
	int m_iHsize;
	float m_ihranges[2];
	const float* m_phranges;
	int m_iVmin;
	int m_iVmax;
	int m_iSmin;

	//����
	int m_ihoughCyclePerfection;

	//����ͼ
	Mat m_cLine; //����·��
	float m_fLineWidth;//·�����
	float m_fMixProportion;//ԭͼ·����ϱ���

						   //β��ͼ
	Mat m_cRoute;//ϸ��
	int m_cRouteWidth;//·�����

					  /*����*/
	int m_iMissMatchCout;//�����CAMSHIFTʧ�ܴ���
};

#endif


