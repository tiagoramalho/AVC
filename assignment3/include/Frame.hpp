#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <opencv2/opencv.hpp>

class Frame{

    public:
        virtual void set_frame_data( uint8_t * frameData) = 0;
        virtual void print_type() = 0;

};

class Frame444: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame444(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        void print_type(){ std::cout << "=== 444 ===" << std::endl; };
};

class Frame422: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame422(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        void print_type(){ std::cout << "=== 422 ===" << std::endl; };
};

class Frame420: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame420(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        void print_type(){ std::cout << "=== 420 ===" << std::endl; };
};

#endif
