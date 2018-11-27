#include <stdio.h>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "Frame.hpp"

using namespace cv;
using namespace std;

static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color)
{
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
            const Point2f& fxy = flow.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x),
    cvRound(y+fxy.y)), color);
            circle(cflowmap, Point(x,y), 2, color, -1);
        }
}

int main(int argc, char** argv)
{

    Frame fr;

    string line; // store the header
    int yCols, yRows; /* frame dimension */
    int fps = 15; /* frames per second */
    int i, n, r, g, b, y, u, v; /* auxiliary variables */
    unsigned char *imgData; // file data buffer
    uchar *buffer; // unsigned char pointer to the Mat data
    char inputKey = '?'; /* parse the pressed key */
    int end = 0, playing = 1, loop = 0; /* control variables */

    /* check for the mandatory arguments */
    if( argc < 2 ) {
        cerr << "Usage: PlayerYUV444 filename" << endl;
        return 1;
    }

    /* Opening video file */
    ifstream myfile (argv[1]);

    /* Processing header */
    getline (myfile,line);
    cout << line << endl;
    cout << line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2) << endl;
    yCols = stoi(line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2));
    yRows = stoi(line.substr(line.find(" H") + 2, line.find(" F") - line.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;

    /* Parse other command line arguments */
    for(n = 1 ; n < argc ; n++)
    {
        if(!strcmp("-fps", argv[n]))
        {
            fps = atof(argv[n+1]);
            n++;
        }

        if(!strcmp("-wait", argv[n]))
        {
            playing = 0;
        }

        if(!strcmp("-loop", argv[n]))
        {
            loop = 1;
        }
    }

    /* data structure for the OpenCv image */
    Mat img = Mat(Size(yCols, yRows), CV_8UC3);

    /* buffer to store the frame */
    imgData = new unsigned char[yCols * yRows * 3];

    /* create a window */
    namedWindow( "rgb");

    // Optical Flow stuff
    // Mat flow, cflow, frame;
    // UMat gray, prevgray, uflow;
    // namedWindow("flow", 1);

    time_t current_time;
    time_t elapsed_time;

    while(!end)
    {
        time (&current_time);

        /* load a new frame, if possible */
        getline (myfile,line); // Skipping word FRAME
        myfile.read((char *)imgData, yCols * yRows * 3);
        if(myfile.gcount() == 0)
        {
            if(loop)
            {
                myfile.clear();
                myfile.seekg(0);
                getline (myfile,line); // read the header
                continue;
            }
            else
            {
                end = 1;
                break;
            }
        }

        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)img.ptr();
        for(i = 0 ; i < yRows * yCols * 3 ; i += 3)
        {
            /* Accessing to planar info */
            y = imgData[i / 3];
            u = imgData[(i / 3) + (yRows * yCols)];
            v = imgData[(i / 3) + (yRows * yCols) * 2];

            /* convert to RGB */
            b = (int)(1.164*(y - 16) + 2.018*(u-128));
            g = (int)(1.164*(y - 16) - 0.813*(u-128) - 0.391*(v-128));
            r = (int)(1.164*(y - 16) + 1.596*(v-128));

            /* clipping to [0 ... 255] */
            if(r < 0) r = 0;
            if(g < 0) g = 0;
            if(b < 0) b = 0;
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;

            /* if you need the inverse formulas */
            //y = r *  .299 + g *  .587 + b *  .114 ;
            //u = r * -.169 + g * -.332 + b *  .500  + 128.;
            //v = r *  .500 + g * -.419 + b * -.0813 + 128.;

            /* Fill the OpenCV buffer - packed mode: BGRBGR...BGR */
            buffer[i] = b;
            buffer[i + 1] = g;
            buffer[i + 2] = r;
        }

        // Optical Flow stuff
        //cvtColor(img, gray, COLOR_BGR2GRAY);
        //if( !prevgray.empty() )
        //{
        //    calcOpticalFlowFarneback(prevgray, gray, uflow,
        //    0.5, 3, 15, 3, 5, 1.2, 0);
        //    cvtColor(prevgray, cflow, COLOR_GRAY2BGR);
        //    uflow.copyTo(flow);
        //    drawOptFlowMap(flow, cflow, 16, 1.5, Scalar(0, 255, 0));
        //    imshow("flow", cflow);
        //}


        time (&elapsed_time);
        double dif = difftime (elapsed_time,current_time);

        /* display the image */
        imshow( "rgb", img );
        if(playing)
        {
            /* wait according to the frame rate */
            inputKey = waitKey(((1.0 / fps) - dif ) * 1000);
        }
        else
        {
            /* wait until user press a key */
            inputKey = waitKey(0);
        }
        /* parse the pressed keys, if any */
        switch((char)inputKey)
        {
            case 'q':
                end = 1;
                break;
            case 'p':
                playing = playing ? 0 : 1;
                break;
        }


        // Optical Flow stuff
        //std::swap(prevgray, gray);
    }
    return 0;
}
