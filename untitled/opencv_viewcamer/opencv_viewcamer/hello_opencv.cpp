#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
	VideoCapture capture(0);
	Mat edges;


	while (1)
	{
		Mat frame;
		capture >> frame;


		imshow("∂¡»° ”∆µ", frame);
		waitKey(30);
	}


	return 0;
}


