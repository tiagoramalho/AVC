#include <stdio.h>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include "../../include/cxxopts.hpp"
#include "../../include/Frame.hpp"
#include "../../include/readerwriterqueue.h"
#include "../../include/atomicops.h"
#include <thread>

using namespace moodycamel;
using namespace cv;
using namespace std;

BlockingReaderWriterQueue<Mat> q(500);

/*
 * Function that parses the Header of the file
 * Returns a map with the values parsed
*/
map<char,string> parse_header(string line, int delimiter(int) = ::isspace )
{

    string token;

    map<char,string> result;

    auto e=line.end();
    auto i=line.begin();

    while(i!=e){
        i=find_if_not(i,e, delimiter);
        if(i==e) break;
        auto j=find_if(i,e, delimiter);
        token = string(i,j);
        cout << token << endl;
        if(token.at(0) == 'W'){
            cout << "Width " << token.substr(1) << endl;
            result['W'] = token.substr(1);
        }
        else if(token.at(0) == 'H'){
            cout << "Height " << token.substr(1) <<endl;
            result['H'] = token.substr(1);
        }
        else if(token.at(0) == 'F'){
            cout << "Frame Rate " << token.substr(1) << endl;
            result['F'] = token.substr(1);
        }
        else if(token.at(0) == 'I'){
            cout << "Interlacing not parsed" << endl;
        }
        else if(token.at(0) == 'A'){
            cout << "Aspect Ratio not parsed" << endl;
        }
        else if(token.at(0) == 'C'){
            cout << "Colour Space " << token.substr(1) << endl;
            result['C'] = token.substr(1);
        }
        i=j;
    }

    return result;
}
void frame_decoding444(ifstream const & file, int const & end, int loop, int yCols, int yRows) {

    int & end_t = const_cast<int &>(end);

    /* Opening video file */
    ifstream & myfile = const_cast<ifstream &>(file);

    string line;

    /* file data buffer */
    unsigned char *imgData;

    /* unsigned char pointer to the Mat data*/
    uchar *buffer;

    while(1){

        /* data structure for the OpenCv image */
        Mat img = Mat(Size(yCols, yRows), CV_8UC3);

        /* buffer to store the frame */
        imgData = new unsigned char[yCols * yRows * 3];

        getline (myfile,line); // Skipping word FRAME

        myfile.read((char *)imgData, yCols * yRows * 3);

        /* data structure to handle frames */
        Frame444 f(yCols, yRows);
        f.set_frame_data(imgData);

        if(myfile.gcount() == 0) {
            if(loop) {
                myfile.clear();
                myfile.seekg(0);
                getline (myfile,line); // read the header continue;
            }
            else {
                end_t= 1;
                break;
            }
        }

        buffer = (uchar*)img.ptr();

        //Uncomment for frame time
        //auto start = std::chrono::high_resolution_clock::now();
        f.get_rgb(buffer);
        //auto finish = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> elapsed = finish - start;
        //std::cout << "Elapsed time: " << elapsed.count() << " s\n";
        q.enqueue(img);
    }
    q.enqueue(Mat());
}

void frame_decoding422(ifstream const & file, int const & end, int loop, int yCols, int yRows) {

    int & end_t = const_cast<int &>(end);
    /* Opening video file */
    ifstream & myfile = const_cast<ifstream &>(file);

    string line;

    /* file data buffer */
    unsigned char *imgData;
    /* unsigned char pointer to the Mat data*/
    uchar *buffer;


    while(1){
        /* data structure for the OpenCv image */
        Mat img = Mat(Size(yCols, yRows), CV_8UC3); 
        /* buffer to store the frame */
        imgData = new unsigned char[yCols * yRows * 2];

        getline (myfile,line); // Skipping word FRAME
        myfile.read((char *)imgData, yCols * yRows * 2);

        /* data structure to handle frames */
        Frame422 f(yCols, yRows);
        f.set_frame_data(imgData);

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
                end_t= 1;
                break;
            }
        }

        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)img.ptr();

        //Uncomment for frame time
        //auto start = std::chrono::high_resolution_clock::now();
        f.get_rgb(buffer);
        //auto finish = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> elapsed = finish - start;
        //std::cout << "Elapsed time: " << elapsed.count() << " s\n";
        q.enqueue(img);

    }
    q.enqueue(Mat());
}

void frame_decoding420(ifstream const & file, int const & end, int loop, int yCols, int yRows)
{

    int & end_t = const_cast<int &>(end);
    /* Opening video file */
    ifstream & myfile = const_cast<ifstream &>(file);

    string line;

    /* file data buffer */
    unsigned char *imgData;
    /* unsigned char pointer to the Mat data*/
    uchar *buffer;


    while(1){
        /* data structure for the OpenCv image */
        Mat img = Mat(Size(yCols, yRows), CV_8UC3);

        /* buffer to store the frame */
        imgData = new unsigned char[yCols * yRows + (yCols * yRows)/2];

        getline (myfile,line); // Skipping word FRAME
        myfile.read((char *)imgData, yCols * yRows + (yCols * yRows)/2);

        /* data structure to handle frames */
        Frame420 f(yCols, yRows);
        f.set_frame_data(imgData);

        if(myfile.gcount() == 0)
        {
            if(loop)
            {
                myfile.clear();
                myfile.seekg(0);
                getline (myfile,line); // read the header continue;
            }
            else
            {
                end_t= 1;
                break;
            }
        }

        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)img.ptr();

        //Uncomment for frame time
        //auto start = std::chrono::high_resolution_clock::now();
        f.get_rgb(buffer);
        //auto finish = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> elapsed = finish - start;
        //std::cout << "Elapsed time: " << elapsed.count() << " s\n";
        q.enqueue(img);
    }
    q.enqueue(Mat());
}

int main(int argc, char** argv)
{
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

    ifstream myfile (file);
    /* store the header */
    string line;
    /* frame dimension */
    int yCols, yRows;
            /* parse the pressed key */
    char inputKey = '?';

    getline(myfile,line);
    map<char,string> header = parse_header(line);


    yCols = stoi(header['W']);
    yRows = stoi(header['H']);

    std::thread t;
    if( header.count('C') != 0){
        if(stoi(header['C']) == 444){
            t = std::thread(frame_decoding444, ref(myfile), ref(end), loop, yCols, yRows);
        }else if(stoi(header['C']) == 422){
            t = std::thread(frame_decoding422, ref(myfile), ref(end), loop, yCols, yRows);
        }
    }else {
        t = std::thread(frame_decoding420, ref(myfile), ref(end), loop, yCols, yRows);
    }

    /* create a window */
    namedWindow( "rgb");


    //  this_thread::sleep_for(chrono::seconds(10));
    Mat img;
    while(1)
    {
        if(img.empty() && end){
            cout << "break" <<endl;
            break;
        }

        if(playing)
        {
            q.wait_dequeue(img);
            ///* display the image */
            imshow( "rgb", img );
            /* wait according to the frame rate */
        auto start = std::chrono::high_resolution_clock::now();
            inputKey = waitKey((1.0 / fps) * 1000);
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "wait key: " << elapsed.count() << " s\n";
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
                cout << "q" <<endl;
                exit(0);
                break;
            case 'p':
                cout << "p" <<endl;
                playing = playing ? 0 : 1;
                break;
        }
    }
    exit(0);
    return 0;
}


