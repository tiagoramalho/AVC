#include <stdio.h>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include "cxxopts.hpp"
#include "Frame.hpp"
#include "readerwriterqueue.h"
#include "atomicops.h"
#include <thread>
#include "Frame.hpp"

using namespace moodycamel;
using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    ReaderWriterQueue<int> q(100);
    /* store the filename */
    string file;
    /* frames per second */
    int fps = 15;
    /* control variables */
    int end = 0, playing = 1, loop = 0;

    try{
        cxxopts::Options options("PSBR", "Player");

        options.add_options()
            ("h,help", "Print help")
            ("w,wait", "Frame Rate")
            ("l,loop", "Loop")
            ("r,framerate", "Frame Rate",cxxopts::value(fps))
            ("f,file", "File (obrigatory)", cxxopts::value<std::string>())
            ;

        auto result = options.parse(argc, argv);

        if (result.count("help")){
            cout << options.help() << endl;
            exit(0);
        }

        if (result.count("f") != 1){
            cout << endl << "You always need to specify a file" << endl << endl;
            cout << options.help() << endl;
            exit(1);
        }else{
            file = result["f"].as<string>();
        }

        if( result.count("r") == 1 )
        {
            cout << "FPS: " << result["r"].as<int>() << endl ;
            fps = result["r"].as<int>();
        }

        if(result.count("w") == 1)
        {
            cout << "Wait" << endl;
            playing = 0;
        }

        if(result.count("l") == 1)
        {
            cout << "Loop" << endl;
            loop = 1;
        }
    }catch(const cxxopts::OptionException& e){
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    Frame fr;

    /* store the header */
    string line;
    /* frame dimension */
    int yCols, yRows;
    /* auxiliary variables */
    int i, n, r, g, b, y, u, v;
    /* file data buffer */
    unsigned char *imgData;
    /* unsigned char pointer to the Mat data*/
    uchar *buffer; 
    /* parse the pressed key */
    char inputKey = '?';

    /* Opening video file */
    ifstream myfile (file);

    /* Processing header */
    getline(myfile,line);
    cout << "Header: " << line << endl;
    string token;
    string delimiter = " ";
    size_t pos = 0;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        if(token.at(0) == 'W'){
            cout << "Width " << token.substr(1) << endl;
            yCols = stoi(token.substr(1));
        }
        else if(token.at(0) == 'H'){
            cout << "Height " << token.substr(1) <<endl;
            yRows = stoi(token.substr(1));
        }
        else if(token.at(0) == 'F'){
            cout << "Frame Rate not parsed" << endl;
        }
        else if(token.at(0) == 'I'){
            cout << "Interlacing not parsed" << endl;
        }
        else if(token.at(0) == 'A'){
            cout << "Aspect Ratio not parsed" << endl;
        }
        else if(token.at(0) == 'C'){
            cout << "Colour Space not parsed" << endl;
        }
        line.erase(0, pos + delimiter.length());
    }

    /* data structure for the OpenCv image */
    Mat img = Mat(Size(yCols, yRows), CV_8UC3);

    /* buffer to store the frame */
    imgData = new unsigned char[yCols * yRows * 3];

    /* create a window */
    namedWindow( "rgb");

    time_t current_time;
    time_t elapsed_time;

    while(!end)
    {
        auto begin = chrono::high_resolution_clock::now();
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

            /* Fill the OpenCV buffer - packed mode: BGRBGR...BGR */
            buffer[i] = b;
            buffer[i + 1] = g;
            buffer[i + 2] = r;
        }

        auto end_time = chrono::high_resolution_clock::now();

        auto dur = end_time - begin;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

        cout << "-->" <<  ms << endl;
        cout << "max" << (1.0 / fps) * 1000 << endl;

        /* display the image */
        imshow( "rgb", img );
        if(playing)
        {
            /* wait according to the frame rate */
            inputKey = waitKey(max((((1.0 / fps)* 1000) - ms), 1.0));
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
