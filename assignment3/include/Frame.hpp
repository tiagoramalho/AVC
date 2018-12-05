#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <opencv2/opencv.hpp>

class Frame {

    public:
        int type, width, height;

        cv::Mat y,u,v;

        Frame(int t, int w, int h): type(t), width(w),height(h){};

        void set_frame_data( uint8_t * frameData);

        void get_rgb( int rgb[] , int line, int col );
};

#endif
