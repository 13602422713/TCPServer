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
	RobotTrack(VideoCapture &cap);//使用此构造函数时，在运行更新的时候必须线调用setTarget()设定目标
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
	/*视频输出*/
	VideoWriter *m_cWriter;
	/**/
	vector<Vec3f> m_vCircles;
	/*camshift*/
	VideoCapture m_pVedio;
	Rect m_cSrcTarget;//源输入
	Mat m_cFrame;	//临时视频帧
	Mat m_cHsv;		//HSV格式
	Mat m_cMask;    //掩码（二值图）
	Mat m_cHue;		//从HSCV中分离出的色相图
	Mat m_cHist;	//色相分布图
	RotatedRect m_cTrackBox;//CamShift追踪结果
	bool m_bFlagOfTrackWindowNeedToSet;//是否需要重新采窗口
	Rect m_cTrackWindow;
	int m_iHsize;
	float m_ihranges[2];
	const float* m_phranges;
	int m_iVmin;
	int m_iVmax;
	int m_iSmin;

	//霍夫
	int m_ihoughCyclePerfection;

	//覆盖图
	Mat m_cLine; //储存路径
	float m_fLineWidth;//路径宽度
	float m_fMixProportion;//原图路径混合比例

						   //尾迹图
	Mat m_cRoute;//细线
	int m_cRouteWidth;//路径宽度

					  /*其他*/
	int m_iMissMatchCout;//霍夫和CAMSHIFT失败次数
};

#endif


