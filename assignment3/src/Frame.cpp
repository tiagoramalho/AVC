#include "Frame.hpp"


void Frame::set_frame_data(uint8_t * frameData){
    if( this->type == 0){
        this->y = cv::Mat(this->height, this->width, CV_8UC1, frameData);
        frameData = frameData + (this->height * this->width);
        this->u = cv::Mat(this->height, this->width, CV_8UC1,frameData);
        frameData = frameData + (this->height * this->width);
        this->v = cv::Mat(this->height, this->width, CV_8UC1,frameData);
    }else{
        exit(1);
    }
}

void Frame::get_rgb( int * rgb, int line, int col){
    uint8_t y,u,v;
    int b,g,r;

    y = this->y.at<uint8_t>(line, col);
    u = this->u.at<uint8_t>(line, col);
    v = this->v.at<uint8_t>(line, col);

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

    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}
