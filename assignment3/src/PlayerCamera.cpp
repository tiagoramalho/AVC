#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
	bool end = false;
	VideoCapture cap;

	if(argc == 2)
	{
		cap = VideoCapture(argv[1]);
	}
	else
	{
		cap = VideoCapture(0);
	}

	if(!cap.isOpened()) return -1;

	Mat frame;

	namedWindow( "rgb");
	while(!end)
	{
		cap >> frame;
		imshow("rgb", frame);
		int key = waitKey(30);
		switch((char)key)
		{
		case 'q':
			end = true;
			break;
		}
	}
	return 0;
}
