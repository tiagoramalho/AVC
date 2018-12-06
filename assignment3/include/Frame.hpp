#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <opencv2/opencv.hpp>

class Frame{};

class Frame444: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame444(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );
};

class Frame422: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame422(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );
};

class Frame420: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame420(int w, int h): width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );
};

#endif
