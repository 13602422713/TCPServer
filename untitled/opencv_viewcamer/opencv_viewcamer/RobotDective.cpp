#include "RobotDective.h"


RobotDective::RobotDective(VideoCapture &cap)
{
	m_pVedio = cap;
	m_ihoughCyclePerfection = 50;
}

RobotDective::~RobotDective()
{

}

bool RobotDective::getRobotPlace(Vec3f &circles, Mat &image)
{
	int ErrCNT = 0;
	while (ErrCNT<10)
	{
		if (getVedioFrame())
		{
			Mat midImage;

			//��3��תΪ�Ҷ�ͼ������ͼ��ƽ��  
			cvtColor(m_cFrame, midImage, CV_BGR2GRAY);//ת����Ե�����ͼΪ�Ҷ�ͼ  
			GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);

			//imshow("midImage", midImage);    

			//��4�����л���Բ�任  
			HoughCircles(midImage, m_vCircles, CV_HOUGH_GRADIENT, 1.5, 10, 10, m_ihoughCyclePerfection, 10, 100);  //cvHoughCircles( CvArr* image, void* circle_storage, int method, double dp, double min_dist, double param1=100, double param2=100, int min_radius=0, int max_radius=0 );
																												   //��5��������ͼ�л��Ƴ�Բ  
			for (size_t i = 0; i < m_vCircles.size(); i++)
			{
				Point center(cvRound(m_vCircles[i][0]), cvRound(m_vCircles[i][1]));
				int radius = cvRound(m_vCircles[i][2]);
				//����Բ��  
				circle(m_cFrame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
				//����Բ����  
				circle(m_cFrame, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			}


			//��6����ʾЧ��ͼ    
			imshow("����Բ", m_cFrame);
			//waitKey(1000);


			//ֻ��ȡ��һ��Բ
			if (1 == m_vCircles.size())
			{
				m_cTarget = m_vCircles[0];
				circles = m_cTarget;
				image = m_cFrame;
				return true;
			}
			else if (1 > m_vCircles.size())
			{
				m_ihoughCyclePerfection = MAX(10, m_ihoughCyclePerfection - 5);
				ErrCNT++;
			}
			else if (1 < m_vCircles.size())
			{
				m_ihoughCyclePerfection = MIN(120, m_ihoughCyclePerfection + 3);
				ErrCNT++;
			}

		}
		else
		{
			ErrCNT++;
		}
	}
	return false;
}

bool RobotDective::getVedioFrame()
{
	Mat frame;
	m_pVedio >> frame;
	if (frame.empty())
		return false;
	frame.copyTo(m_cFrame);
}
