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
	//设定临时参数
	m_pVedio = cap;
}

bool RobotTrack::setTarget(Rect target)
{
	m_cSrcTarget = target;

	//camShift相关参数
	m_iVmin = 10;
	m_iVmax = 100;
	m_iSmin = 10;
	m_iHsize = 16;
	m_ihranges[0] = 0;
	m_ihranges[1] = 180;
	m_phranges = m_ihranges;

	//Houg相关
	m_ihoughCyclePerfection = 50;

	//线路相关参数 ！！！！！缺获取失败处理！！！！！！
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
		m_cRoute = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);//细线
		m_cRoute = Scalar::all(0);
		m_cRouteWidth = 8;//路径宽度
	}

	//其他
	m_iMissMatchCout = 0;

	m_bFlagOfTrackWindowNeedToSet = true;
	return true;
}
RobotTrack::RobotTrack(VideoCapture &cap, Rect target)
{
	//设定临时参数
	m_pVedio = cap;
	m_cSrcTarget = target;

	//camShift相关参数
	m_iVmin = 10;
	m_iVmax = 100;
	m_iSmin = 10;
	m_iHsize = 16;
	m_ihranges[0] = 0;
	m_ihranges[1] = 180;
	m_phranges = m_ihranges;

	//Houg相关
	m_ihoughCyclePerfection = 50;

	//线路相关参数 ！！！！！缺获取失败处理！！！！！！
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
		m_cRoute = Mat::zeros(m_cFrame.rows, m_cFrame.cols, CV_8UC3);//细线
		m_cRoute = Scalar::all(0);
		m_cRouteWidth = 5;//路径宽度
	}

	//其他
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
//刷新位置
bool RobotTrack::updateRobotPlace(Vec3f &robotCycle)
{
	if (!isTrackSrcDidSet())
	{
		throw("Track srouce didn't set.");
	}
	if (getVedioFrame())
	{
		/*将image转换为HSV格式 HSV=hue\saturation\value(色相、饱和度、明度)*/
		cvtColor(m_cFrame, m_cHsv, COLOR_BGR2HSV);

		//int _vmin = vmin, _vmax = vmax;

		/*二值化处理，判断hsv 是否在_vmin _vmax之间，二值图输出到mask*/
		inRange(m_cHsv, Scalar(0, m_iSmin, MIN(m_iVmin, m_iVmax)),
			Scalar(180, 256, MAX(m_iVmin, m_iVmax)), m_cMask);

		int ch[] = { 0, 0 };
		/*以hsv图的大小创建hue*/
		m_cHue.create(m_cHsv.size(), m_cHsv.depth());

		/*四通道图像分割 */
		/*-----------------------------------------------------------------*/
		//CV_EXPORTS void mixChannels(
		//		const Mat* src,//源图像
		//		size_t nsrcs, //输入矩阵的数量
		//		Mat* dst,//输出数组或矩阵向量
		//		size_t ndsts,//输出矩阵的数量
		//      const int* fromTo,//指定被赋值通道与要赋值到的位置组成的索引对 
		//		size_t npairs);//fromTo中索引对的数目
		/*-------------------------------------------------------------------*/
		mixChannels(&m_cHsv, 1, &m_cHue, 1, ch, 1);


		if (isTackWindowNeedReset())
		{
			m_bFlagOfTrackWindowNeedToSet = false;
			resetTrackWindow();
		}

		/*反向投影，储存在backproj中*/
		Mat backproj;
		/*-----------------------------------------------------------------*/
		//CV_EXPORTS void calcBackProject( 
		//				const Mat* images,	//源图
		//				int nimages,		//输入图像的数量
		//				const int* channels,//用于计算反向投影的通道列表，通道数必须与直方图维度相匹配
		//				InputArray hist,	//输入的直方图，直方图的bin可以是密集或者稀疏
		//				OutputArray backProject,//目标反向投影输出图像，是一个单通道图像，与原图像有相同的尺寸和深度
		//				const float** ranges,//直方图中每个维度bin的取值范围
		//				double scale=1,		//可选输出反向投影的比例因子
		//				bool uniform=true );//直方图是否均匀分布的标识符
		/*-----------------------------------------------------------------*/
		calcBackProject(&m_cHue, 1, 0, m_cHist, backproj, (const float **)&m_phranges);

		/*使用mask作为掩码取出backproj的mask区域*/
		backproj &= m_cMask;

		/*CamShift*/
		m_cTrackBox = CamShift(
			backproj,	//输入图像
			m_cTrackWindow,//输入和输出的搜索窗口/目标窗口
			TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));//迭代收敛终止条件
																	 /*---------------------------------------------------------------------*/
																	 //TermCriteria::TermCriteria(int _type, int _maxCount, double _epsilon)
																	 //类型：ermCriteria::COUNT（迭代到最大迭代次数）、TermCriteria::EPS（迭代到阈值终止）、TermCriteria::COUNT+EPS（两者都作为迭代终止条件）
																	 //迭代的最大次数
																	 //特定阈值
																	 /*---------------------------------------------------------------------*/

																	 /*如果trackWindow面积过小，重新计算trackWindow*/
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
		//生成一个比m_cTrackWindow大一倍的空间houghZone作ROI，在里边使用霍夫变换,  //！！！！！此处需要添加Mat边界溢出保护！！！！
		int r = MAX(m_cTrackWindow.height, m_cTrackWindow.width) / 2;
		int hougZoneX = MAX(0, m_cTrackWindow.x - r);
		int hougZoneY = MAX(0, m_cTrackWindow.y - r);
		int hougZoneWidth = MIN(3 * r, m_cFrame.cols - hougZoneX);
		int hougZoneHeight = MIN(3 * r, m_cFrame.rows - hougZoneY);
		Rect houghZone(hougZoneX, hougZoneY, hougZoneWidth, hougZoneHeight);
		Mat midImage;
		Mat roi(m_cFrame, houghZone);
		//转为灰度图，进行图像平滑  
		cvtColor(roi, midImage, CV_BGR2GRAY);//转化边缘检测后的图为灰度图  
		GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);

		//imshow("midImage", midImage);    

		//进行霍夫圆变换  
		HoughCircles(midImage, m_vCircles, CV_HOUGH_GRADIENT, 1.5, 10, m_ihoughCyclePerfection, 100, (int)(m_cSrcTarget.height / 4), (int)(m_cSrcTarget.height / 2 * 1.5));  //cvHoughCircles( CvArr* image, void* circle_storage, int method, double dp, double min_dist, double param1=100, double param2=100, int min_radius=0, int max_radius=0 );

		for (size_t i = 0; i < m_vCircles.size(); i++)
		{
			m_vCircles[i][0] += houghZone.x;
			m_vCircles[i][1] += houghZone.y;
		}

		////依次在图中绘制出圆 //测试
		//for( size_t i = 0; i < m_vCircles.size(); i++ )  
		//{  
		//	Point center(cvRound(m_vCircles[i][0]), cvRound(m_vCircles[i][1]));  
		//	int radius = cvRound(m_vCircles[i][2]);  
		//	//绘制圆心  
		//	circle( m_cFrame, center, 3, Scalar(0,255,0), -1, 8, 0 );  
		//	//绘制圆轮廓  
		//	circle( m_cFrame, center, radius, Scalar(155,50,255), 3, 8, 0 );  
		//} 

		//筛选最好的位置
		Vec3f bestCycle;		//最好的位置	
		selectoverlappingCycle(m_vCircles, m_cTrackBox);
		if (!selectRadiusBestCycle(m_vCircles, bestCycle))
		{
			//绘制连线()
			mixRouteAndLine();
			//显示CamShift轨迹
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

		static Vec3f lastBestCycle = bestCycle;	//上次的位置

												//圆心和半径
		Point center(cvRound(bestCycle[0]), cvRound(bestCycle[1]));
		Point lastCenter(cvRound(lastBestCycle[0]), cvRound(lastBestCycle[1]));
		int radius = cvRound(bestCycle[2]);
		//绘制圆心  
		circle(m_cFrame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		//绘制圆轮廓  
		circle(m_cFrame, center, radius, Scalar(155, 50, 255), 3, 8, 0);
		//绘制覆盖区和轨迹()
		line(m_cLine, center, lastCenter, Scalar(0, 200, 200), (int)(m_cSrcTarget.height*m_fLineWidth), 8, 0);
		line(m_cRoute, center, lastCenter, Scalar(255, 200, 0), m_cRouteWidth, 8, 0);
		mixRouteAndLine();
		//更新上次位置
		lastBestCycle = bestCycle;

		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/
		/*---------------------------------------------------------------------*/

		///*绘制椭圆，圈出trackBox*/
		//      ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );

		//显示CamShift的轨迹
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

//显示轨迹
bool RobotTrack::showTrack()
{
	/*绘制椭圆，圈出trackBox*/
	ellipse(m_cFrame, m_cTrackBox, Scalar(0, 0, 255), 3, CV_AA);

	imshow("showTrack", m_cFrame);



	return true;
}

//输出视频流 缺少保护
bool RobotTrack::ouput()
{
	return true;
}

//输出视频流
Mat RobotTrack::getLast()
{
	return m_cFrame;
}

//筛选重叠部分
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
//选出半径最接近源图的圆
bool RobotTrack::selectRadiusBestCycle(vector<Vec3f> &vec, Vec3f &cycle)
{
	if (vec.empty())
	{
		return false;
	}
	static Vec3f lastCycle = vec[0];

	//设置第一个圆为最优
	Point center(cvRound(vec[0][0]), cvRound(vec[0][1]));
	int radius = cvRound(vec[0][2]);
	int lastRadius = (int)lastCycle[2];
	//int err = abs(radius - m_cSrcTarget.height);
	int err = abs(radius - lastRadius);
	Vec3f bestCycle = vec[0];
	int bestErro = err;
	//找更优
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
	//均值滤波
	bestCycle[0] = (bestCycle[0] + lastCycle[0]) / 2;
	bestCycle[1] = (bestCycle[1] + lastCycle[1]) / 2;
	bestCycle[2] = (bestCycle[2] + lastCycle[2]) / 2;

	cycle = bestCycle;
	lastCycle = bestCycle;
	return true;
}

//重置追踪窗口
bool RobotTrack::resetTrackWindow()
{
	/*用选定的区域selection设定ROI区域*/
	Mat roi(m_cHue, m_cSrcTarget);
	Mat maskroi(m_cMask, m_cSrcTarget);
	imshow("roi", roi);


	/*直方图计算，储存到hist*/
	calcHist(&roi,
		1,			//多少张图像序列中的图像用于计算直方图
		0,			//输入的每一张图像有可能是多通道的
		maskroi,	//mask掩膜，指定哪些像素被用于计算直方图
		m_cHist,		//保存直方图的结果矩阵
		1,			//需要计算的直方图维数
		&m_iHsize,     //需要计算直方图的每一维的大小，每一维bin的个数
		&m_phranges);

	/*归一*/
	normalize(m_cHist, m_cHist, 0, 255, CV_MINMAX);

	/*设置追踪窗口为selection的部分*/
	m_cTrackWindow = m_cSrcTarget;
	m_bFlagOfTrackWindowNeedToSet = false;

	/*设置histimg中所有元素为0*/
	Mat histimg = Mat::zeros(200, 320, CV_8UC3);
	histimg = Scalar::all(0);

	/**/
	int binW = histimg.cols / m_iHsize;

	/*创建一个8位无符号3通道图像*/
	Mat buf(1, m_iHsize, CV_8UC3);
	for (int i = 0; i < m_iHsize; i++)
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / m_iHsize), 255, 255);

	/*换成HSV格式*/
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

//获取视频帧
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
