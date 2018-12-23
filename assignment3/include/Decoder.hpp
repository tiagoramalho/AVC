#ifndef DECODER_H
#define DECODER_H

#include "Golomb.hpp"
#include "Frame.hpp"
#include <opencv2/opencv.hpp>


using namespace cv;


class Decoder {

    private:
        /* Temporário */

        double dataLuminance[8][8] = {
            {16, 11, 10, 16, 24, 40, 51, 61},
            {12, 12, 14, 19, 26, 58, 60, 55},
            {14, 13, 16, 24, 40, 57, 69, 56},
            {14, 17, 22, 29, 51, 87, 80, 62},
            {18, 22, 37, 56, 68, 109, 103, 77},
            {24, 35, 55, 64, 81, 104, 113, 92},
            {49, 64, 78, 87, 103, 121, 120, 101},
            {72, 92, 95, 98, 112, 100, 103, 99}
        };

        Mat mat_luminance = Mat(8, 8, CV_64FC1, &dataLuminance);

        double dataChrominance[8][8] = {
            {17, 18, 24, 27, 99, 99, 99, 99},
            {18, 21, 26, 66, 99, 99, 99, 99},
            {24, 26, 56, 99, 99, 99, 99, 99},
            {47, 66, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99}
        };

        Mat mat_chrominance = Mat(8, 8, CV_64FC1, &dataChrominance);


        int height, width, color_space;
        ofstream outfile;
        READBits r;
        int profile, periodicity, block_size, search_area;


        // void parse_header_pv(map<char,string> & header, string header_line, int delimiter(int) = ::isspace);
        
        uint8_t get_real_value_uniform(uint8_t prevision, int residual);
        uint8_t get_real_value_LOCO( uint8_t pixel_A, uint8_t pixel_B, uint8_t pixel_C, int residual);


        void write_header_y4m(uint32_t width, uint32_t height, string fps, uint32_t colorspace){
            if(colorspace == 420){
                outfile << "YUV4MPEG2" << " W" << width << " H" << height << " F" << fps << " Ip A1:1"<< endl;
            }else{
                outfile << "YUV4MPEG2" << " W" << width << " H" << height << " F" << fps << " Ip A1:1 C"<< colorspace << endl;
            }
        }


        vector<Point> get_vectors(int k, Golomb & g);


        void write_header_frame(){
            outfile << "FRAME" << endl;
        }

    public:

        Decoder(const string & in_file, const string & out_file);

        void read_and_decode_lossy();
        Mat decode_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix);
        void decode_intra_lossy(Frame * frame, Golomb & g, Golomb & g_zeros, int frame_matrix);
        void decode_non_intra_lossy(Frame * frame, Frame * previous_frame, Golomb & g, Golomb & g_zeros, int frame_matrix);

        /*  Function used to encode and write N Frames intra-mode  */
        void decode_intra(Frame * frame, uint8_t seed,int k, Golomb & g, uint8_t type);

        /*  Function used to encode and write N Frames  inter-mode  */
        void decode_inter(Frame * current_frame , Frame * last_frame,
            int k, Golomb & g, uint8_t type, vector<Point> & vectors, int shamnt);

        /* Function used to encode and write */
        void read_and_decode();
};

#endif
