#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <opencv2/opencv.hpp>

class Frame{

    public:
        cv::Mat y,u,v;
        virtual void set_frame_data( uint8_t * frameData) = 0;
        virtual int print_type() = 0;
        virtual cv::Mat get_y() = 0;
        virtual cv::Mat get_u() = 0;
        virtual cv::Mat get_v() = 0;

        virtual ~Frame(){

        }

};

class Frame444: public Frame {

    public:
        int  width, height;

        Frame444(int h, int w): width(w),height(h){
            this->y = cv::Mat(this->height, this->width, CV_8UC1);
            this->u = cv::Mat(this->height, this->width, CV_8UC1);
            this->v = cv::Mat(this->height, this->width, CV_8UC1);
        };

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        int print_type(){ std::cout << "=== 444 ===" << std::endl; return 444; };

        cv::Mat get_y(){return this->y;};
        cv::Mat get_u(){return this->u;};
        cv::Mat get_v(){return this->v;};
};

class Frame422: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame422(int h, int w): width(w),height(h){
            this->y = cv::Mat(this->height, this->width, CV_8UC1);
            this->u = cv::Mat(this->height, this->width/2, CV_8UC1);
            this->v = cv::Mat(this->height, this->width/2, CV_8UC1);
        };

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        int print_type(){ std::cout << "=== 422 ===" << std::endl; return 422; };

        cv::Mat get_y(){return this->y;};
        cv::Mat get_u(){return this->u;};
        cv::Mat get_v(){return this->v;};

};

class Frame420: public Frame {

    public:
        int  width, height;

        cv::Mat y,u,v;

        Frame420(int h, int w): width(w),height(h){
            this->y = cv::Mat(this->height, this->width, CV_8UC1);
            this->u = cv::Mat(this->height/2, this->width/2, CV_8UC1);
            this->v = cv::Mat(this->height/2, this->width/2, CV_8UC1);
        };

        void set_frame_data( uint8_t * frameData);

        void get_rgb( uchar *buffer );

        int print_type(){ std::cout << "=== 420 ===" << std::endl; return 420;};

        cv::Mat get_y(){return this->y;};
        cv::Mat get_u(){return this->u;};
        cv::Mat get_v(){return this->v;};
};

#endif
