#include "../include/Frame.hpp"

//  Frame 444 functions
 
void Frame444::set_frame_data(uint8_t * frameData){
    this->y = cv::Mat(this->height, this->width, CV_8UC1, frameData);
    frameData = frameData + (this->height * this->width);
    this->u = cv::Mat(this->height, this->width, CV_8UC1,frameData);
    frameData = frameData + (this->height * this->width);
    this->v = cv::Mat(this->height, this->width, CV_8UC1,frameData);
}

void Frame444::get_rgb( uchar * buffer){
    uint8_t y,u,v;
    int b,g,r, indexer, lines, cols;


    for( lines = 0; lines < height; lines+=1){
        uint8_t *yline = this->y.ptr(lines);
        uint8_t *uline = this->u.ptr(lines);
        uint8_t *vline = this->v.ptr(lines);
        for( cols = 0; cols < width; cols+=1){


            y = yline[cols];
            u = uline[cols];
            v = vline[cols];

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

            indexer = (cols*3) + (width * lines * 3);
            buffer[ indexer ]     = b;
            buffer[ indexer + 1]  = g;
            buffer[ indexer + 2]  = r;

        }
    }
}
//  Frame 422 functions

void Frame422::set_frame_data(uint8_t * frameData){
    this->y = cv::Mat(this->height, this->width, CV_8UC1, frameData);
    frameData = frameData + (this->height * this->width);
    this->u = cv::Mat(this->height, this->width/2, CV_8UC1,frameData);
    frameData = frameData + (this->height * this->width/2);
    this->v = cv::Mat(this->height, this->width/2, CV_8UC1,frameData);
}

void Frame422::get_rgb( uchar * buffer){
    uint8_t y,u,v;
    int b,g,r, indexer, lines, cols;


    for( lines = 0; lines < height; lines+=1){
        uint8_t *yline = this->y.ptr(lines);
        uint8_t *uline = this->u.ptr(lines);
        uint8_t *vline = this->v.ptr(lines);
        for( cols = 0; cols < width; cols+=1){


            y = yline[cols];
            u = uline[cols/2];
            v = vline[cols/2];

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

            indexer = (cols*3) + (width * lines * 3);
            buffer[ indexer ]     = b;
            buffer[ indexer + 1]  = g;
            buffer[ indexer + 2]  = r;

        }
    }
}
//  Frame 420 functions
void Frame420::set_frame_data(uint8_t * frameData){
    this->y = cv::Mat(this->height, this->width, CV_8UC1, frameData);
    frameData = frameData + (this->height * this->width);
    this->u = cv::Mat(this->height/2, this->width/2, CV_8UC1,frameData);
    frameData = frameData + (this->height/2 * this->width/2);
    this->v = cv::Mat(this->height/2, this->width/2, CV_8UC1,frameData);
}

void Frame420::get_rgb( uchar * buffer){
    uint8_t y,u,v;
    int b,g,r, indexer, lines, cols;


    for( lines = 0; lines < height; lines+=1){
        uint8_t *yline = this->y.ptr(lines);
        uint8_t *uline = this->u.ptr(lines/2);
        uint8_t *vline = this->v.ptr(lines/2);
        for( cols = 0; cols < width; cols+=1){


            y = yline[cols];
            u = uline[cols/2];
            v = vline[cols/2];

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

            indexer = (cols*3) + (width * lines * 3);
            buffer[ indexer ]     = b;
            buffer[ indexer + 1]  = g;
            buffer[ indexer + 2]  = r;

        }
    }
}
