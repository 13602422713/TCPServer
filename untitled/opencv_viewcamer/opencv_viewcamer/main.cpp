#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "RobotDective.h"
#include "RobotTrack.h"
#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

#define STEP_FindRobot 1
#define STEP_Track 2
char step = STEP_FindRobot;


static void help()
{
	cout << "Airob Robot Track project\n";
}

const char* keys =
{
	"{1|  | 0 | camera number}"
};

//int main(int argc, const char** argv)
int main()
{
	/*显示提示信息*/
	help();

	VideoCapture cap;
	Rect trackWindow;

	//开启摄像头/录像
	//CommandLineParser parser(argc, argv, keys);
	//   int camNum = parser.get<int>("0");


	//   cap.open(camNum);
	cap.open(0);
	//cap.open("Test7.mp4");

	if (!cap.isOpened())
	{
		help();
		cout << "***Could not initialize capturing...***\n";
		cout << "Current parameter's value: \n" ;
	//	parser.printParams();
		return -1;
	}

	//开启输出保存
	Mat frame;
	cap >> frame;
	VideoWriter writer("VideoTest.avi", CV_FOURCC('D', 'I', 'V', 'X'), 20, Size(frame.cols, frame.rows));


	Rect selection;

	Mat image;
	Vec3f Target;
	RobotDective robotdective(cap);
	RobotTrack robotTrack(cap);
	Vec3f robotCycle;//本次机器人圆
	while (1)
	{
		switch (step)
		{
		case STEP_FindRobot:
			for (;;)
			{
				if (robotdective.getRobotPlace(Target, image))
				{
					//追踪
					int CycleX = cvRound(Target[0]);
					int CycleY = cvRound(Target[1]);
					int radius = cvRound(Target[2]);
					selection.x = CycleX - radius;
					selection.y = CycleY - radius;
					selection.width = 2 * radius;
					selection.height = 2 * radius;
					selection &= Rect(0, 0, image.cols, image.rows);
					step = STEP_Track;
					break;
				}
			}
			break;
		case STEP_Track:
			robotTrack.setTarget(selection);
			for (;;)
			{
				try
				{
					if (!robotTrack.updateRobotPlace(robotCycle))
					{
						step = STEP_FindRobot;
						break;
						//重新进行STEP1；
					}
					else
					{
						Mat frame = robotTrack.getLast();
						writer << frame;
					}
				}
				catch (char *ch)
				{

				}
				char c = (char)waitKey(10);
				if (c == 27)
					break;
			}
			break;
			//  default:
			//	step = STEP_FindRobot;
			break;
		}
	}
	system("pause");
}


