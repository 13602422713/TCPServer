#ifndef __ROBOTDECTIVE_H
#define __ROBOTDECTIVE_H

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>


using namespace cv;
using namespace std;


class RobotDective
{
public:
	RobotDective(VideoCapture &cap);
	~RobotDective();
	bool getRobotPlace(Vec3f &circles, Mat &image);
private:
	bool getVedioFrame();
private:
	vector<Vec3f> m_vCircles;
	VideoCapture m_pVedio;
	Mat m_cFrame;
	Vec3f m_cTarget;
	int m_ihoughCyclePerfection;
};


#endif