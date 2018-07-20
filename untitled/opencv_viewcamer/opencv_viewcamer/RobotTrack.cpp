#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "RobotTrack.h"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;



RobotTrack::RobotTrack(VideoCapture &cap)
{
	//�趨��ʱ����
	m_pVedio = cap;
}

bool RobotTrack::setTarget(Rect target)
{
	m_cSrcTarget = target;

	//camShift��ز���
	m_iVmin = 10;
	m_iVmax = 100;
	m_iSmin = 10;
	m_iHsize = 16;
	m_ihranges[0] = 0;
	m_ihranges[1] = 180;
	m_phranges = m_ihranges;

	//Houg���
	m_ihoughCyclePerfection = 50;

	//��·��ز��� ����������ȱ��ȡʧ�ܴ�������������
	if (0 == m_cLine.rows)
	{
		m_pVedio >> m_cFrame;
		m_cLine = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);
		m_cLine = Scalar::all(0);
		m_fLineWidth = 0.95f;
		m_fMixProportion = 0.7f;
	}
	if (0 == m_cRoute.rows)
	{
		m_pVedio >> m_cFrame;
		m_cRoute = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);//ϸ��
		m_cRoute = Scalar::all(0);
		m_cRouteWidth = 8;//·�����
	}

	//����
	m_iMissMatchCout = 0;

	m_bFlagOfTrackWindowNeedToSet = true;
	return true;
}
RobotTrack::RobotTrack(VideoCapture &cap, Rect target)
{
	//�趨��ʱ����
	m_pVedio = cap;
	m_cSrcTarget = target;

	//camShift��ز���
	m_iVmin = 10;
	m_iVmax = 100;
	m_iSmin = 10;
	m_iHsize = 16;
	m_ihranges[0] = 0;
	m_ihranges[1] = 180;
	m_phranges = m_ihranges;

	//Houg���
	m_ihoughCyclePerfection = 50;

	//��·��ز��� ����������ȱ��ȡʧ�ܴ�������������
	if (0 == m_cLine.rows)
	{
		m_pVedio >> m_cFrame;
		m_cLine = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);
		m_cLine = Scalar::all(0);
		m_fLineWidth = 0.95f;
		m_fMixProportion = 0.7f;
	}
	if (0 == m_cRoute.rows)
	{
		m_pVedio >> m_cFrame;
		m_cRoute = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);//ϸ��
		m_cRoute = Scalar::all(0);
		m_cRouteWidth = 5;//·�����
	}

	//����
	m_iMissMatchCout = 0;

	m_bFlagOfTrackWindowNeedToSet = true;
}


bool RobotTrack::setOutput(VideoWriter &writer)
{
	m_cWriter = &writer;
	return true;
}

RobotTrack::~RobotTrack()
{

}
//ˢ��λ��
bool RobotTrack::updateRobotPlace(Vec3f &robotCycle)
{
	if (!isTrackSrcDidSet())
	{
		throw("Track srouce didn't set.");
	}
	if (getVedioFrame())
	{
		/*��imageת��ΪHSV��ʽ HSV=hue\saturation\value(ɫ�ࡢ���Ͷȡ�����)*/
		cvtColor(m_cFrame, m_cHsv, COLOR_BGR2HSV);

		//int _vmin = vmin, _vmax = vmax;

		/*��ֵ�������ж�hsv �Ƿ���_vmin _vmax֮�䣬��ֵͼ�����mask*/
		inRange(m_cHsv, Scalar(0, m_iSmin, MIN(m_iVmin, m_iVmax)),
			Scalar(180, 256, MAX(m_iVmin, m_iVmax)), m_cMask);

		int ch[] = { 0, 0 };
		/*��hsvͼ�Ĵ�С����hue*/
		m_cHue.create(m_cHsv.size(), m_cHsv.depth());

		/*��ͨ��ͼ��ָ� */
		/*-----------------------------------------------------------------*/
		//CV_EXPORTS void mixChannels(
		//		const Mat* src,//Դͼ��
		//		size_t nsrcs, //������������
		//		Mat* dst,//���������������
		//		size_t ndsts,//������������
		//      const int* fromTo,//ָ������ֵͨ����Ҫ��ֵ����λ����ɵ������� 
		//		size_t npairs);//fromTo�������Ե���Ŀ
		/*-------------------------------------------------------------------*/
		mixChannels(&m_cHsv, 1, &m_cHue, 1, ch, 1);


		if (isTackWindowNeedReset())
		{
			m_bFlagOfTrackWindowNeedToSet = false;
			resetTrackWindow();
		}

		/*����ͶӰ��������backproj��*/
		Mat backproj;
		/*-----------------------------------------------------------------*/
		//CV_EXPORTS void calcBackProject( 
		//				const Mat* images,	//Դͼ
		//				int nimages,		//����ͼ�������
		//				const int* channels,//���ڼ��㷴��ͶӰ��ͨ���б�ͨ����������ֱ��ͼά����ƥ��
		//				InputArray hist,	//�����ֱ��ͼ��ֱ��ͼ��bin�������ܼ�����ϡ��
		//				OutputArray backProject,//Ŀ�귴��ͶӰ���ͼ����һ����ͨ��ͼ����ԭͼ������ͬ�ĳߴ�����
		//				const float** ranges,//ֱ��ͼ��ÿ��ά��bin��ȡֵ��Χ
		//				double scale=1,		//��ѡ�������ͶӰ�ı�������
		//				bool uniform=true );//ֱ��ͼ�Ƿ���ȷֲ��ı�ʶ��
		/*-----------------------------------------------------------------*/
		calcBackProject(&m_cHue, 1, 0, m_cHist, backproj, (const float **)&m_phranges);

		/*ʹ��mask��Ϊ����ȡ��backproj��mask����*/
		backproj &= m_cMask;

		/*CamShift*/
		m_cTrackBox = CamShift(
			backproj,	//����ͼ��
			m_cTrackWindow,//������������������/Ŀ�괰��
			TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));//����������ֹ����
																	 /*---------------------------------------------------------------------*/
																	 //TermCriteria::TermCriteria(int _type, int _maxCount, double _epsilon)
																	 //���ͣ�ermCriteria::COUNT����������������������TermCriteria::EPS����������ֵ��ֹ����TermCriteria::COUNT+EPS�����߶���Ϊ������ֹ������
																	 //������������
																	 //�ض���ֵ
																	 /*---------------------------------------------------------------------*/

																	 /*���trackWindow�����С�����¼���trackWindow*/
		if (m_cTrackWindow.area() <= 10)
		{
			//int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
			//m_cTrackWindow = Rect(m_cTrackWindow.x - r, m_cTrackWindow.y - r,
			//                    m_cTrackWindow.x + r, m_cTrackWindow.y + r) &
			//                Rect(0, 0, cols, rows);
			return false;
		}

		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		//����һ����m_cTrackWindow��һ���Ŀռ�houghZone��ROI�������ʹ�û���任,  //�����������˴���Ҫ���Mat�߽����������������
		int r = MAX(m_cTrackWindow.height, m_cTrackWindow.width) / 2;
		int hougZoneX = MAX(0, m_cTrackWindow.x - r);
		int hougZoneY = MAX(0, m_cTrackWindow.y - r);
		int hougZoneWidth = MIN(3 * r, m_cFrame.cols - hougZoneX);
		int hougZoneHeight = MIN(3 * r, m_cFrame.rows - hougZoneY);
		Rect houghZone(hougZoneX, hougZoneY, hougZoneWidth, hougZoneHeight);
		Mat midImage;
		Mat roi(m_cFrame, houghZone);
		//תΪ�Ҷ�ͼ������ͼ��ƽ��  
		cvtColor(roi, midImage, CV_BGR2GRAY);//ת����Ե�����ͼΪ�Ҷ�ͼ  
		GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);

		//imshow("midImage", midImage);    

		//���л���Բ�任  
		HoughCircles(midImage, m_vCircles, CV_HOUGH_GRADIENT, 1.5, 10, m_ihoughCyclePerfection, 100, (int)(m_cSrcTarget.height / 4), (int)(m_cSrcTarget.height / 2 * 1.5));  //cvHoughCircles( CvArr* image, void* circle_storage, int method, double dp, double min_dist, double param1=100, double param2=100, int min_radius=0, int max_radius=0 );

		for (size_t i = 0; i < m_vCircles.size(); i++)
		{
			m_vCircles[i][0] += houghZone.x;
			m_vCircles[i][1] += houghZone.y;
		}

		////������ͼ�л��Ƴ�Բ //����
		//for( size_t i = 0; i < m_vCircles.size(); i++ )  
		//{  
		//	Point center(cvRound(m_vCircles[i][0]), cvRound(m_vCircles[i][1]));  
		//	int radius = cvRound(m_vCircles[i][2]);  
		//	//����Բ��  
		//	circle( m_cFrame, center, 3, Scalar(0,255,0), -1, 8, 0 );  
		//	//����Բ����  
		//	circle( m_cFrame, center, radius, Scalar(155,50,255), 3, 8, 0 );  
		//} 

		//ɸѡ��õ�λ��
		Vec3f bestCycle;		//��õ�λ��	
		selectoverlappingCycle(m_vCircles, m_cTrackBox);
		if (!selectRadiusBestCycle(m_vCircles, bestCycle))
		{
			//��������()
			mixRouteAndLine();
			//��ʾCamShift�켣
			showTrack();

			m_iMissMatchCout++;
			waitKey(10);

			if (m_iMissMatchCout>50)
			{
				m_iMissMatchCout = 0;
				return false;
			}
			else
			{
				m_ihoughCyclePerfection = MAX(10, m_ihoughCyclePerfection - 3);
				return true;
			}
		}

		static Vec3f lastBestCycle = bestCycle;	//�ϴε�λ��

												//Բ�ĺͰ뾶
		Point center(cvRound(bestCycle[0]), cvRound(bestCycle[1]));
		Point lastCenter(cvRound(lastBestCycle[0]), cvRound(lastBestCycle[1]));
		int radius = cvRound(bestCycle[2]);
		//����Բ��  
		circle(m_cFrame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		//����Բ����  
		circle(m_cFrame, center, radius, Scalar(155, 50, 255), 3, 8, 0);
		//���Ƹ������͹켣()
		line(m_cLine, center, lastCenter, Scalar(0, 200, 200), (int)(m_cSrcTarget.height*m_fLineWidth), 8, 0);
		line(m_cRoute, center, lastCenter, Scalar(255, 200, 0), m_cRouteWidth, 8, 0);
		mixRouteAndLine();
		//�����ϴ�λ��
		lastBestCycle = bestCycle;

		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/

		///*������Բ��Ȧ��trackBox*/
		//      ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );

		//��ʾCamShift�Ĺ켣
		showTrack();

		robotCycle = bestCycle;
		return true;
	}
	return false;
}
int RobotTrack::mixRouteAndLine()
{
	Mat mid;
	addWeighted(m_cLine, 0.3, m_cRoute, 0.7, 0, mid);
	addWeighted(mid, 1 - m_fMixProportion, m_cFrame, m_fMixProportion, 0, m_cFrame);
	return 0;
}

//��ʾ�켣
bool RobotTrack::showTrack()
{
	/*������Բ��Ȧ��trackBox*/
	ellipse(m_cFrame, m_cTrackBox, Scalar(0, 0, 255), 3, CV_AA);

	imshow("showTrack", m_cFrame);



	return true;
}

//�����Ƶ�� ȱ�ٱ���
bool RobotTrack::ouput()
{
	return true;
}

//�����Ƶ��
Mat RobotTrack::getLast()
{
	return m_cFrame;
}

//ɸѡ�ص�����
bool RobotTrack::selectoverlappingCycle(vector<Vec3f> &vec, RotatedRect trackbox)
{
	vector<Vec3f> vecRsult;
	for (size_t i = 0; i < vec.size(); i++)
	{
		Point center(cvRound(vec[i][0]), cvRound(vec[i][1]));
		int radius = cvRound(vec[i][2]);
		if (sqrt((trackbox.center.x - center.x)*(trackbox.center.x - center.x) + (trackbox.center.y - center.y)*(trackbox.center.y - center.y))< radius + MAX(trackbox.size.height, trackbox.size.width))
			vecRsult.push_back(vec[i]);
	}
	if (vecRsult.empty())
		return false;

	vec = vecRsult;
	return true;
}
//ѡ���뾶��ӽ�Դͼ��Բ
bool RobotTrack::selectRadiusBestCycle(vector<Vec3f> &vec, Vec3f &cycle)
{
	if (vec.empty())
	{
		return false;
	}
	static Vec3f lastCycle = vec[0];

	//���õ�һ��ԲΪ����
	Point center(cvRound(vec[0][0]), cvRound(vec[0][1]));
	int radius = cvRound(vec[0][2]);
	int lastRadius = (int)lastCycle[2];
	//int err = abs(radius - m_cSrcTarget.height);
	int err = abs(radius - lastRadius);
	Vec3f bestCycle = vec[0];
	int bestErro = err;
	//�Ҹ���
	for (size_t i = 1; i < vec.size(); i++)
	{
		Point center(cvRound(vec[i][0]), cvRound(vec[i][1]));
		int radius = cvRound(vec[i][2]);
		//int err = abs(radius - m_cSrcTarget.height);
		int err = abs(radius - lastRadius);
		if (err < bestErro)
		{
			bestCycle = vec[i];
			bestErro = err;
		}
	}
	//��ֵ�˲�
	bestCycle[0] = (bestCycle[0] + lastCycle[0]) / 2;
	bestCycle[1] = (bestCycle[1] + lastCycle[1]) / 2;
	bestCycle[2] = (bestCycle[2] + lastCycle[2]) / 2;

	cycle = bestCycle;
	lastCycle = bestCycle;
	return true;
}

//����׷�ٴ���
bool RobotTrack::resetTrackWindow()
{
	/*��ѡ��������selection�趨ROI����*/
	Mat roi(m_cHue, m_cSrcTarget);
	Mat maskroi(m_cMask, m_cSrcTarget);
	imshow("roi", roi);


	/*ֱ��ͼ���㣬���浽hist*/
	calcHist(&roi,
		1,			//������ͼ�������е�ͼ�����ڼ���ֱ��ͼ
		0,			//�����ÿһ��ͼ���п����Ƕ�ͨ����
		maskroi,	//mask��Ĥ��ָ����Щ���ر����ڼ���ֱ��ͼ
		m_cHist,		//����ֱ��ͼ�Ľ������
		1,			//��Ҫ�����ֱ��ͼά��
		&m_iHsize,     //��Ҫ����ֱ��ͼ��ÿһά�Ĵ�С��ÿһάbin�ĸ���
		&m_phranges);

	/*��һ*/
	normalize(m_cHist, m_cHist, 0, 255, CV_MINMAX);

	/*����׷�ٴ���Ϊselection�Ĳ���*/
	m_cTrackWindow = m_cSrcTarget;
	m_bFlagOfTrackWindowNeedToSet = false;

	/*����histimg������Ԫ��Ϊ0*/
	Mat histimg = Mat::zeros(200, 320, CV_8UC3);
	histimg = Scalar::all(0);

	/**/
	int binW = histimg.cols / m_iHsize;

	/*����һ��8λ�޷���3ͨ��ͼ��*/
	Mat buf(1, m_iHsize, CV_8UC3);
	for (int i = 0; i < m_iHsize; i++)
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / m_iHsize), 255, 255);

	/*����HSV��ʽ*/
	cvtColor(buf, buf, CV_HSV2BGR);

	for (int i = 0; i < m_iHsize; i++)
	{
		int val = saturate_cast<int>(m_cHist.at<float>(i)*histimg.rows / 255);
		rectangle(histimg, Point(i*binW, histimg.rows),
			Point((i + 1)*binW, histimg.rows - val),
			Scalar(buf.at<Vec3b>(i)), -1, 8);
	}
	return true;
}

//��ȡ��Ƶ֡
bool RobotTrack::getVedioFrame()
{
	Mat frame;
	m_pVedio >> frame;
	if (frame.empty())
		return false;
	frame.copyTo(m_cFrame);
	return true;
}

bool RobotTrack::isTackWindowNeedReset()
{
	return m_bFlagOfTrackWindowNeedToSet;
}


bool RobotTrack::isTrackSrcDidSet()
{
	if (0 == m_cSrcTarget.width)
	{
		return false;
	}
	return true;
}
